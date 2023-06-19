#ifndef APP_INCLUDE_APP_DRIVERS_GESTURE_H_
#define APP_INCLUDE_APP_DRIVERS_GESTURE_H_

#include <errno.h>
#include <zephyr/device.h>

enum gesture_type {
    GESTURE_UP,
    GESTURE_DOWN,
    GESTURE_LEFT,
    GESTURE_RIGHT,
    GESTURE_FORWARD,
    GESTURE_BACKWARD,
    GESTURE_NONE,
};

typedef void (*gesture_trigger_handler_t)(const struct device * dev);

typedef int (*gesture_get_t)(const struct device * dev,
                             enum gesture_type * gesture);
typedef int (*gesture_trigger_set_t)(const struct device * dev,
                                     gesture_trigger_handler_t handler);

__subsystem struct gesture_driver_api {
    gesture_get_t get;
    gesture_trigger_set_t trigger_set;
};

__syscall int gesture_get(const struct device * dev, enum gesture_type * gesture);

static inline int z_impl_gesture_get(const struct device * dev,
                                     enum gesture_type * gesture) {
    const struct gesture_driver_api * api =
        (const struct gesture_driver_api *) dev->api;

    return api->get(dev, gesture);
}

static inline int gesture_trigger_set(const struct device * dev,
                                      gesture_trigger_handler_t handler) {
    const struct gesture_driver_api * api =
        (const struct gesture_driver_api *) dev->api;

    if (api->trigger_set == NULL) {
        return -ENOSYS;
    }

    return api->trigger_set(dev, handler);
}

#define GESTURE_DEVICE_DT_DEFINE(node_id, init_fn, pm_device, data_ptr, \
                                 cfg_ptr, level, prio, api_ptr, ...)    \
    DEVICE_DT_DEFINE(node_id, init_fn, pm_device, data_ptr, cfg_ptr,    \
                     level, prio, api_ptr, __VA_ARGS__)

#define GESTURE_DEVICE_DT_INST_DEFINE(inst, ...)    \
    GESTURE_DEVICE_DT_DEFINE(DT_DRV_INST(inst), __VA_ARGS__)

#include <syscalls/gesture.h>

#endif
