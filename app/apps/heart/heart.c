#define APP_NAME heart
#include "app/base.h"
#include "heart.h"

lv_obj_t *label;

const struct device *heart_dev = DEVICE_DT_GET(DT_CHOSEN(nerw_heart));

struct k_sem read_bpm_finished;
struct heart_work heart_work;

K_THREAD_STACK_DEFINE(heart_reader_stack_area, 1024);
static struct k_thread heart_reader_data;

void heart_reader(void *arg1, void *arg2, void *arg3) {
    ARG_UNUSED(arg1); ARG_UNUSED(arg2); ARG_UNUSED(arg3);

    k_work_submit(&heart_work.work);
    /* k_condvar_(&read_bpm_finished, K_SECONDS(20)); */

    /* TODO: Write to file */

    k_sleep(K_MINUTES(20));
}

void setup(struct app *app) {
    printk("Hello from heart\n");

    if (heart_dev == NULL)
        LOG_ERR("cannot get device: max30101");

    if (!device_is_ready(heart_dev))
        LOG_ERR("device is nor ready: max30101");

    k_work_init(&heart_work.work, get_bpm);
    k_sem_init(&read_bpm_finished, 0, 1);

    k_thread_create(
        &heart_reader_data, heart_reader_stack_area,
        K_THREAD_STACK_SIZEOF(heart_reader_stack_area),
        heart_reader, NULL, NULL, NULL,
        7, 0, K_FOREVER);

    /* k_thread_start(&heart_reader_data); */
    
    label = lv_label_create(app->screen);
    lv_label_set_text(label, "BPM: ");
    lv_obj_center(label);
}

extern struct tm extern_time;

void loop(struct app *app) {
    k_work_submit(&heart_work.work);
    if (k_sem_take(&read_bpm_finished, K_MSEC(10)) == 0) {
        lv_label_set_text_fmt(label, "BPM: %d", heart_work.bpm);
    } else {
        k_work_cancel(&heart_work.work);
        lv_label_set_text(label, "BPM: measuring");        
    }
    k_sleep(K_MSEC(10));
}

void icon_create(struct app *app) {
    lv_obj_t *label;

    label = lv_label_create(app->icon);
    lv_label_set_text(label, "Heart");
    lv_obj_center(label);
}
