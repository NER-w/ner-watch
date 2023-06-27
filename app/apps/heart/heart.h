#ifndef HEART_APP_H_
#define HEART_APP_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>

extern const struct device *heart_dev;

struct heart_work {
    struct k_work work;
    uint16_t bpm;
};

extern struct heart_work heart_work;
extern struct k_sem read_bpm_finished;

void get_bpm(struct k_work *work);

#endif /* HEART_APP_H_ */
