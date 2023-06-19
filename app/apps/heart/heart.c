#define APP_NAME heart
#define APP_STACK_SIZE 8192
#include "app/base.h"

lv_obj_t *label;

void setup(struct app *app) {
    printk("Hello from heart\n");

    label = lv_label_create(app->screen);
    lv_label_set_text(label, "Be healthy, so you won't see an unfamiliar ceiling");
    lv_obj_center(label);
}

void loop(struct app *app) {
    k_sleep(K_MSEC(1000));
}
