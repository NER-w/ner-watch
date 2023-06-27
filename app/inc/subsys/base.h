#ifndef SUBSYS_BASE_H_
#define SUBSYS_BASE_H_

#include <stddef.h>
#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(SUBSYS_NAME, CONFIG_LOG_DEFAULT_LEVEL);

#define SUBSYS_STACK_(name) _CONCAT(name, _stack_area)
#define SUBSYS_STACK SUBSYS_STACK_(SUBSYS_NAME)

#define SUBSYS_THREAD_(name) _CONCAT(name, _thread)
#define SUBSYS_THREAD SUBSYS_THREAD_(SUBSYS_NAME)

#define SUBSYS_REGISTER_(name) _CONCAT(name, _register)
#define SUBSYS_REGISTER SUBSYS_REGISTER_(SUBSYS_NAME)

#define SUBSYS_INIT_(name) _CONCAT(name, _init)
#define SUBSYS_INIT SUBSYS_INIT_(SUBSYS_NAME)

#define SUBSYS_INIT_FN_(name) _CONCAT(name, _fn)
#define SUBSYS_INIT_FN SUBSYS_INIT_FN_(SUBSYS_NAME)

#ifndef SUBSYS_STACK_SIZE
#define SUBSYS_STACK_SIZE 2048
#endif
#define SUBSYS_PRIORITY 9

#define SUBSYS_NAME_STR STRINGIFY(SUBSYS_NAME)

#include <subsys/list.h>

K_THREAD_STACK_DEFINE(SUBSYS_STACK, SUBSYS_STACK_SIZE);
static struct k_thread SUBSYS_THREAD;

static int init(void);

int SUBSYS_REGISTER(void) {
    subsys_count++;
    LOG_INF("register: subsys count: %d", subsys_count);
    return 0;
}

int SUBSYS_INIT_FN(void *arg1, void *arg2, void *arg3) {
    ARG_UNUSED(arg1); ARG_UNUSED(arg2); ARG_UNUSED(arg3);

    return init();
}

int SUBSYS_INIT(void) {
    subsys_list[subsys_index].name   = SUBSYS_NAME_STR;
    subsys_list[subsys_index].thread = &SUBSYS_THREAD;

    k_thread_create(
        &SUBSYS_THREAD, SUBSYS_STACK,
        K_THREAD_STACK_SIZEOF(SUBSYS_STACK),
        SUBSYS_INIT_FN, NULL, NULL, NULL,
        SUBSYS_PRIORITY, 0, K_FOREVER);

    subsys_index++;
    return 0;
}

SYS_INIT(SUBSYS_REGISTER, APPLICATION, 1);
SYS_INIT(SUBSYS_INIT, APPLICATION, 3);

#endif /* SUBSYS_BASE_H_ */
