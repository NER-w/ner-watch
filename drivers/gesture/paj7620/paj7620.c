#define DT_DRV_COMPAT pixart_paj7620

#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/pm/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#include <app/drivers/gesture.h>

#include "paj7620.h"

LOG_MODULE_REGISTER(PAJ7620, CONFIG_GESTURE_LOG_LEVEL);

static void paj7620_handle_cb(struct paj7620_data * drv_data) {
    paj7620_setup_int(drv_data->dev->config, false);

#ifdef CONFIG_PAJ7620_TRIGGER
    k_work_submit(&drv_data->work);
#else
    k_sem_give(&drv_data->data_sem);
#endif
}

static void paj7620_gpio_callback(const struct device * dev,
                                  struct gpio_callback * cb,
                                  uint32_t pins)
{
    struct paj7620_data * drv_data =
        CONTAINER_OF(cb, struct paj7620_data, gpio_cb);

    paj7620_handle_cb(drv_data);
}

static int paj7620_get(const struct device * dev,
                       enum gesture_type * gesture) {
    const struct paj7620_config * config = dev->config;

    uint8_t data = 0;
    if (i2c_reg_write_byte_dt(&config->i2c, PAJ7620_BANK_SEL_REG, PAJ7620_BANK0)) {
        LOG_ERR("Failed to select bank");
        return -EIO;
    }
    if (i2c_reg_read_byte_dt(&config->i2c, PAJ7620_GESTURE_REG, &data)) {
        LOG_ERR("Failed to read gesture");
        return -EIO;
    }

    LOG_DBG("Gesture data %d", data);
    
    switch (data) {
    case PAJ7620_GESTURE_UP:
        *gesture = GESTURE_UP;
        break;
    case PAJ7620_GESTURE_DOWN:
        *gesture = GESTURE_DOWN;
        break;
    case PAJ7620_GESTURE_LEFT:
        *gesture = GESTURE_LEFT;
        break;
    case PAJ7620_GESTURE_RIGHT:
        *gesture = GESTURE_RIGHT;
        break;
    case PAJ7620_GESTURE_FORWARD:
        *gesture = GESTURE_FORWARD;
        break;
    case PAJ7620_GESTURE_BACKWARD:
        *gesture = GESTURE_BACKWARD;
        break;
    default:
        *gesture = GESTURE_NONE;
    }
    return 0;
}

static int paj7620_sensor_setup(const struct device * dev) {
    const struct paj7620_config * config = dev->config;
    uint16_t chip_id;

    if (i2c_reg_read_byte_dt(&config->i2c,
                             PAJ7620_ID_PART1_REG, (uint8_t *) &chip_id) ||
        i2c_reg_read_byte_dt(&config->i2c,
                             PAJ7620_ID_PART2_REG, ((uint8_t *) &chip_id) + 1)) {
        LOG_ERR("Failed reading chip id");
        return -EIO;
    }
        
    if (chip_id != PAJ7620_ID) {
        LOG_ERR("Invalid chip id 0x%x", chip_id);
        return -EIO;
    }

    const size_t setup_instr_size = 110;
    const uint8_t setup_instr[] = {
        0xEF, 0x00,       // Bank 0
        0x41, 0x00,       // Disable interrupts for first 8 gestures
        0x42, 0x00,       // Disable wave (and other modes') interrupt(s)
        0x37, 0x07,
        0x38, 0x17,
        0x39, 0x06,
        0x42, 0x01,
        0x46, 0x2D,
        0x47, 0x0F,
        0x48, 0x3C,
        0x49, 0x00,
        0x4A, 0x1E,
        0x4C, 0x22,
        0x51, 0x10,
        0x5E, 0x10,
        0x60, 0x27,
        0x80, 0x42,
        0x81, 0x44,
        0x82, 0x04,
        0x8B, 0x01,
        0x90, 0x06,
        0x95, 0x0A,
        0x96, 0x0C,
        0x97, 0x05,
        0x9A, 0x14,
        0x9C, 0x3F,
        0xA5, 0x19,
        0xCC, 0x19,
        0xCD, 0x0B,
        0xCE, 0x13,
        0xCF, 0x64,
        0xD0, 0x21,
        0xEF, 0x01,       // Bank 1
        0x02, 0x0F,
        0x03, 0x10,
        0x04, 0x02,
        0x25, 0x01,
        0x27, 0x39,
        0x28, 0x7F,
        0x29, 0x08,
        0x3E, 0xFF,
        0x5E, 0x3D,
        0x65, 0x96,       // R_IDLE_TIME LSB - Set sensor speed to 'normal speed' - 120 fps
        0x67, 0x97,
        0x69, 0xCD,
        0x6A, 0x01,
        0x6D, 0x2C,
        0x6E, 0x01,
        0x72, 0x01,
        0x73, 0x35,
        0x74, 0x00,       // Set to gesture mode
        0x77, 0x01,
        0xEF, 0x00,       // Bank 0
        0x41, 0xFF,       // Re-enable interrupts for first 8 gestures
        0x42, 0x01        // Re-enable interrupts for wave gesture
    };
    for (size_t i = 0; i < setup_instr_size; i += 2) {
        if (i2c_reg_write_byte_dt(&config->i2c, setup_instr[i], setup_instr[i + 1])) {
            LOG_ERR("Failed writing setup instruction %zu", i / 2);
            return -EIO;
        }
    }

    const size_t gesture_instr_size = 58;
    const uint8_t gesture_instr[] = {
        0xEF, 0x00,       // Bank 0
        0x41, 0x00,       // Disable interrupts for first 8 gestures
        0x42, 0x00,       // Disable wave (and other mode's) interrupt(s)
        0x48, 0x3C,
        0x49, 0x00,
        0x51, 0x10,
        0x83, 0x20,
        0x9f, 0xf9,
        0xEF, 0x01,       // Bank 1
        0x01, 0x1E,
        0x02, 0x0F,
        0x03, 0x10,
        0x04, 0x02,
        0x41, 0x40,
        0x43, 0x30,
        0x65, 0x96,       // R_IDLE_TIME  - Normal mode LSB "120 fps" (supposedly)
        0x66, 0x00,
        0x67, 0x97,
        0x68, 0x01,
        0x69, 0xCD,
        0x6A, 0x01,
        0x6b, 0xb0,
        0x6c, 0x04,
        0x6D, 0x2C,
        0x6E, 0x01,
        0x74, 0x00,       // Set gesture mode
        0xEF, 0x00,       // Bank 0
        0x41, 0xFF,       // Re-enable interrupts for first 8 gestures
        0x42, 0x01,       // Re-enable interrupts for wave gesture
    };
    for (size_t i = 0; i < gesture_instr_size; i+= 2) {
        if (i2c_reg_write_byte_dt(&config->i2c, gesture_instr[i], gesture_instr[i + 1])) {
            LOG_ERR("Failed writing gesture instruction %zu", i / 2);
            return -EIO;
        }
    }

    return 0;
}

static int paj7620_init_interrupt(const struct device * dev) {
    const struct paj7620_config * config = dev->config;
    struct paj7620_data * drv_data = dev->data;

    if (!device_is_ready(config->int_gpio.port)) {
        LOG_ERR("%s: device %s is not ready",
                dev->name, config->int_gpio.port->name);
        return -ENODEV;
    }

    int err;
    if ((err = gpio_pin_configure_dt(&config->int_gpio, GPIO_INPUT | config->int_gpio.dt_flags)) < 0) {
        LOG_ERR("%s: failed to configure pin: %d: %s", dev->name, err, strerror(-err));
        return err;
    }

    gpio_init_callback(&drv_data->gpio_cb,
                       paj7620_gpio_callback,
                       BIT(config->int_gpio.pin)
        );

    if (gpio_add_callback(config->int_gpio.port, &drv_data->gpio_cb) < 0) {
        LOG_DBG("Failed to set gpio callback!");
        return -EIO;
    }

    drv_data->dev = dev;

#ifdef CONFIG_PAJ7620_TRIGGER
    drv_data->work.handler = paj7620_work_cb;

#else
    k_sem_init(&drv_data->data_sem, 0, K_SEM_MAX_LIMIT);
#endif
    
    paj7620_setup_int(config, true);

    if (gpio_pin_get_dt(&config->int_gpio) > 0) {
        paj7620_handle_cb(drv_data);
    }

    return 0;
}

#ifdef CONFIG_PM_DEVICE
static int paj7620_pm_action(const struct device * dev,
                             enum pm_device_action action)
{
    const struct paj7620_config * config = dev->config;
    int ret = 0;

    switch (action) {
    case PM_DEVICE_ACTION_RESUME:
        if () {
            ret = -EIO
        }

        break;
    default:
        return -ENOTSUP;
    }

    return ret;
}
#endif

static int paj7620_init(const struct device * dev) {
    const struct paj7620_config * config = dev->config;
    struct paj7620_data * data = dev->data;

    /* Wait for the device initialization */
    k_sleep(K_MSEC(7));

    if (!device_is_ready(config->i2c.bus)) {
        LOG_ERR("Bus device is not ready");
        return -EINVAL;
    }
    LOG_DBG("I2C bus is ready");

    for (int i = 0; i < 2; i++) {
        i2c_reg_write_byte_dt(&config->i2c, PAJ7620_BANK_SEL_REG, PAJ7620_BANK0);
    }
    
    if (paj7620_sensor_setup(dev) < 0) {
        LOG_ERR("Failed to setup device");
        return -EIO;
    }
    LOG_DBG("Setup completed");

    if (paj7620_init_interrupt(dev) < 0) {
        LOG_ERR("Failed to initialize interrupt");
        return -EIO;
    }
    LOG_DBG("Interrupt initialized");

    return 0;
}

static const struct gesture_driver_api paj7620_driver_api = {
    .get = &paj7620_get,
#ifdef CONFIG_PAJ7620_TRIGGER
    .trigger_set = &paj7620_trigger_set,
#endif
};

static const struct paj7620_config paj7620_config = {
    .i2c = I2C_DT_SPEC_INST_GET(0),
    .int_gpio = GPIO_DT_SPEC_INST_GET(0, int_gpios),
    
};

static struct paj7620_data paj7620_data;

PM_DEVICE_DT_INST_DEFINE(0, paj7620_pm_action);

GESTURE_DEVICE_DT_INST_DEFINE(
    0, paj7620_init,
    PM_DEVICE_DT_INST_GET(0), &paj7620_data, &paj7620_config,
    POST_KERNEL, CONFIG_GESTURE_INIT_PRIORITY, &paj7620_driver_api);

