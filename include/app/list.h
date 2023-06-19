#ifndef APP_LIST_H_
#define APP_LIST_H_

#include <zephyr/kernel.h>

#include <lvgl.h>

struct app {
    const char *name;
    struct k_thread *thread;
    lv_obj_t *screen;
};

extern struct app *app_list;
extern size_t app_count;
extern size_t app_index;

#endif
