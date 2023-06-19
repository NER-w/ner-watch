#ifndef APP_DRIVERS_GESTURE_PAJ7620_PAJ7620_H_
#define APP_DRIVERS_GESTURE_PAJ7620_PAJ7620_H_

#include <zephyr/drivers/gpio.h>

#define PAJ7620_BANK_SEL_REG 0xEF
#define PAJ7620_BANK0        0x00
#define PAJ7620_BANK1        0x01

#define PAJ7620_ID_PART1_REG 0x00
#define PAJ7620_ID_PART2_REG 0x01
#define PAJ7620_ID           0x7620

#define PAJ7620_GESTURE_REG 0x43
#define PAJ7620_GESTURE_UP 0x01
#define PAJ7620_GESTURE_DOWN 0x02
#define PAJ7620_GESTURE_LEFT 0x04
#define PAJ7620_GESTURE_RIGHT 0x08
#define PAJ7620_GESTURE_FORWARD 0x10
#define PAJ7620_GESTURE_BACKWARD 0x20

struct paj7620_config {
    struct i2c_dt_spec i2c;
    struct gpio_dt_spec int_gpio;
};

struct paj7620_data {
    struct gpio_callback gpio_cb;
    struct k_work work;
    const struct device * dev;
    enum gesture_type gesture;

#ifdef CONFIG_PAJ7620_TRIGGER
    gesture_trigger_handler_t p_th_handler;
#else
    struct k_sem data_sem;
#endif
};

static inline void paj7620_setup_int(const struct paj7620_config * cfg, bool enable) {
    unsigned int flags = enable ? GPIO_INT_EDGE_TO_ACTIVE : GPIO_INT_DISABLE;

    gpio_pin_interrupt_configure_dt(&cfg->int_gpio, flags);
}

#ifdef CONFIG_PAJ7620_TRIGGER
void paj7620_work_cb(struct k_work * work);

int paj7620_trigger_set(const struct device * dev,
                        gesture_trigger_handler_t handler);
#endif

#endif /* APP_DRIVERS_GESTURE_PAJ7620_PAJ7620_H_ */
