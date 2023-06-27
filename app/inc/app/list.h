#ifndef APP_LIST_H_
#define APP_LIST_H_

#include <zephyr/kernel.h>

#include <lvgl.h>

struct app {
    const char *name;
    struct k_thread *thread;
    
    lv_obj_t *screen;
    lv_obj_t *icon;

    void (*icon_create)(struct app *app);
};

extern lv_obj_t *app_select_screen;

extern struct app *app_list;
extern size_t app_count;
extern size_t app_index;

#endif
