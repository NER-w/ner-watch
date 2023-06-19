#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#include <app/drivers/gesture.h>

#include "paj7620.h"

extern struct paj7620_data paj7620_driver;

LOG_MODULE_DECLARE(PAJ7620, CONFIG_GESTURE_LOG_LEVEL);

void paj7620_work_cb(struct k_work * work) {
    struct paj7620_data * data = CONTAINER_OF(work, struct paj7620_data, work);
    const struct device * dev = data->dev;

    if (data->p_th_handler != NULL) {
        data->p_th_handler(dev);
    }

    paj7620_setup_int(dev->config, true);
}

int paj7620_trigger_set(const struct device * dev,
                        gesture_trigger_handler_t handler) {
    const struct paj7620_config * config = dev->config;
    struct paj7620_data * data = dev->data;

    paj7620_setup_int(config, false);

    data->p_th_handler = handler;

    paj7620_setup_int(config, true);
    if (gpio_pin_get_dt(&config->int_gpio) > 0) {
        k_work_submit(&data->work);
    }

    return 0;
}
