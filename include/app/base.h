#ifndef APP_BASE_H_
#define APP_BASE_H_

#include <stddef.h>
#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/toolchain.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(APP_NAME, CONFIG_LOG_DEFAULT_LEVEL);

#include <lvgl.h>

#define APP_MAIN_FN_(name) _CONCAT(name, _main)
#define APP_MAIN_FN APP_MAIN_FN_(APP_NAME)

#define APP_STACK_(name) _CONCAT(name, _stack_area)
#define APP_STACK APP_STACK_(APP_NAME)

#define APP_THREAD_(name) _CONCAT(name, _thread)
#define APP_THREAD APP_THREAD_(APP_NAME)

#define APP_REGISTER_(name) _CONCAT(name, _register)
#define APP_REGISTER APP_REGISTER_(APP_NAME)

#define APP_INIT_(name) _CONCAT(name, _init)
#define APP_INIT APP_INIT_(APP_NAME)

#ifndef APP_STACK_SIZE
#define APP_STACK_SIZE 4096
#endif
#define APP_PRIORITY 7

#define APP_NAME_STR STRINGIFY(APP_NAME)

#include "app/list.h"

K_THREAD_STACK_DEFINE(APP_STACK, APP_STACK_SIZE);
static struct k_thread APP_THREAD;

static void setup(struct app *app);
static void loop(struct app *app);

void APP_MAIN_FN(void *app_arg, void *arg1, void *arg2) {
    ARG_UNUSED(arg1); ARG_UNUSED(arg2);
    
    struct app *app = (struct app *) app_arg;
    app->screen = lv_obj_create(NULL);
    setup(app);
    lv_scr_load(app->screen);
    lv_task_handler();
    
    while (-42) {
        if (app->screen != lv_scr_act())
            lv_scr_load(app->screen);
        loop(app);
        lv_task_handler();
    }
}

int APP_REGISTER(void) {
    app_count++;
    LOG_INF("register: app count %d", app_count);
    return 0;
}

int APP_INIT(void) {
    app_list[app_index].name = APP_NAME_STR;
    app_list[app_index].thread = &APP_THREAD;

    int err;
    err = k_thread_create(
        &APP_THREAD, APP_STACK,
        K_THREAD_STACK_SIZEOF(APP_STACK),
        APP_MAIN_FN, &app_list[app_index], NULL, NULL,
        APP_PRIORITY, 0, K_FOREVER);
    if (err < 0) {
        LOG_ERR("init: failed to create thread: %d: %s",
                err,
                strerror(-err)
            );
        return err;
    }
    app_index++;
    #ifdef CONFIG_THREAD_MONITOR
    err = k_thread_name_set(&APP_THREAD, APP_NAME_STR);
    if (err < 0) {
        LOG_ERR("init: failed to set name: %d: %s",
                err,
                strerror(-err)
            );
        return err;
    }
    #endif
    LOG_INF("init: Ok");
    return 0;
}

SYS_INIT(APP_REGISTER, APPLICATION, 1);
SYS_INIT(APP_INIT, APPLICATION, 3);

#endif /* APP_BASE_H_ */
