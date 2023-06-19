/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/smf.h>
#include <zephyr/drivers/display.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, CONFIG_LOG_DEFAULT_LEVEL);

#include <app/drivers/gesture.h>
#include <lvgl.h>

#include <app/list.h>

#define EVENT_GESTURE_UP    BIT(0)
#define EVENT_GESTURE_DOWN  BIT(1)
#define EVENT_GESTURE_LEFT  BIT(2)
#define EVENT_GESTURE_RIGHT BIT(3)
#define EVENT_GESTURE_TO    BIT(4)
#define EVENT_GESTURE_FROM  BIT(5)

#define EVENT_GESTURES      EVENT_GESTURE_UP    | EVENT_GESTURE_DOWN | \
                            EVENT_GESTURE_RIGHT | EVENT_GESTURE_LEFT |  \
                            EVENT_GESTURE_TO    | EVENT_GESTURE_FROM

#define EVENT_BUTTON_PRESS BIT(6)

#define EVENT_ALL EVENT_GESTURES | EVENT_BUTTON_PRESS

static const struct smf_state app_states[];
enum watch_state { SLEEP, APP, SELECT };

struct s_object {
    struct smf_ctx ctx;

    struct k_event smf_event;
    int32_t events;

    size_t app_index;
    struct app * app;
} s_obj;

static void sleep_entry(void *o) {
    /* display_ */
}

static void sleep_exit(void *o) {
}

static void app_entry(void *o) {
    struct s_object *obj = (struct s_object *) o;
    
    printk("App entry %d\n", obj->app_index);
    obj->app = &app_list[obj->app_index];
    /* printk("App name: %s\n", app_list[app_ */
    /* printk("0x%p\n", obj->app); */
    /* printk("%s\n", obj->app->name); */

    k_thread_resume(obj->app->thread);
}

static void app_run(void *o) {
    int32_t events = ((struct s_object *) o)->events;

    if (events & EVENT_BUTTON_PRESS) {
        smf_set_state(SMF_CTX(&s_obj), &app_states[SLEEP]);
    } else if (events & EVENT_GESTURE_TO) {
        smf_set_state(SMF_CTX(&s_obj), &app_states[SELECT]);
    } else if (events & EVENT_GESTURE_DOWN) {
        printk("App scroll down\n");
    } else {
        
    }
}

static void app_exit(void *o) {
    struct s_object *obj = (struct s_object *) o;

    k_thread_suspend(obj->app->thread);
}

static void select_run(void *o) {
    struct s_object *obj = (struct s_object *) o;
    int32_t events = obj->events;

    if (events & EVENT_BUTTON_PRESS) {
        smf_set_state(SMF_CTX(&s_obj), &app_states[SLEEP]);
    } else if (events & EVENT_GESTURE_FROM) {
        smf_set_state(SMF_CTX(&s_obj), &app_states[APP]);
    } else if (events & EVENT_GESTURE_LEFT) {
        printk("Selector swipe left\n");
        obj->app_index++;
        obj->app_index %= app_count;
    } else if (events & EVENT_GESTURE_RIGHT) {
        printk("Selector swipe right\n");
        obj->app_index--;
        obj->app_index %= app_count;
    } else {
    }    
}

static const struct smf_state app_states[] = {
    [SLEEP]  = SMF_CREATE_STATE(sleep_entry, NULL, sleep_exit, NULL),
    [APP]    = SMF_CREATE_STATE(app_entry, app_run, app_exit, NULL),
    [SELECT] = SMF_CREATE_STATE(NULL, select_run, NULL, NULL),
};

int main(void) {
	const struct device *display_dev;
    
	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Display is not ready");
		return 0;
	}

    LOG_INF("app list allocated: %s", app_list == NULL ? "false" : "true");
    LOG_INF("app list size: %zu", app_count);

    /* struct app clock; */
    size_t clock_app_index = 0;

    for (size_t i = 0; i < app_count; i++) {
        if (strncmp(app_list[i].name, "clock", 5) == 0) {
            printk("Clock app at index %zu\n", i);
            clock_app_index = i;
            /* clock = app_list[i]; */
        }

        k_thread_start(app_list[i].thread);
        k_thread_suspend(app_list[i].thread);
    }
    
    s_obj.app_index = clock_app_index;

    int32_t ret;
    k_event_init(&s_obj.smf_event);

    smf_set_initial(SMF_CTX(&s_obj), &app_states[APP]);

	display_blanking_off(display_dev);
    
    while (1) {
        s_obj.events = k_event_wait(&s_obj.smf_event, EVENT_GESTURES, true, K_FOREVER);
        if (s_obj.events)
            printk("Events: 0x%x\n", s_obj.events);

        ret = smf_run_state(SMF_CTX(&s_obj));
        if (ret) {
            LOG_ERR("state machine results in error: %d", ret);
            break;
        }

        /* k_sleep(K_MSEC(100)); */
    }

    /* k_thread_start(clock.thread); */

    return 0;
    /* lv_scr_load(clock.screen); */

	/* lv_task_handler(); */
}

void gesture_handler(const struct device *dev) {
    enum gesture_type gesture = 0;
    
    gesture_get(dev, &gesture);
    switch (gesture) {
    case GESTURE_UP:
        printk("Up\n");
        k_event_post(&s_obj.smf_event, EVENT_GESTURE_UP);
        break;
    case GESTURE_DOWN:
        printk("Down\n");        
        k_event_post(&s_obj.smf_event, EVENT_GESTURE_DOWN);
        break;
    case GESTURE_LEFT:
        printk("Left\n");
        k_event_post(&s_obj.smf_event, EVENT_GESTURE_LEFT);
        break;
    case GESTURE_RIGHT:
        printk("Right\n");
        k_event_post(&s_obj.smf_event, EVENT_GESTURE_RIGHT);
        break;
    case GESTURE_FORWARD:
        printk("To\n");
        k_event_post(&s_obj.smf_event, EVENT_GESTURE_TO);
        break;
    case GESTURE_BACKWARD:
        printk("From\n");
        k_event_post(&s_obj.smf_event, EVENT_GESTURE_FROM);
        break;
    case GESTURE_NONE:
        break;
    }
}

K_THREAD_STACK_DEFINE(gesture_stack_area, 1024);
static struct k_thread gesture_data;

/* threadA is a static thread that is spawned automatically */

void gesture_reader(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

    const struct device *gesture_dev;

	gesture_dev = DEVICE_DT_GET(DT_CHOSEN(nerw_gesture));
	if (!device_is_ready(gesture_dev)) {
		LOG_ERR("Gesture sensor is not ready");
		return 0;
	}

	while (1)
	{
		gesture_handler(gesture_dev);
		k_msleep(100);
	}

}

K_THREAD_DEFINE(gesture_tid, 1024,
                gesture_reader, NULL, NULL, NULL,
                7, 0, 0);
