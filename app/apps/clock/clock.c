#define APP_NAME clock
#define APP_STACK_SIZE 2304
#include "app/base.h"

#include <time.h>

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>

static lv_obj_t *meter;
static lv_obj_t *label;
static lv_meter_indicator_t *indic_min;
static lv_meter_indicator_t *indic_hour;

void setup(struct app *app) {
    printk("Hello from clock\n");

    meter = lv_meter_create(app->screen);
    lv_obj_set_size(meter, 240, 240);
    lv_obj_center(meter);

    lv_meter_scale_t *scale_min, *scale_hour;

    scale_min = lv_meter_add_scale(meter);
	lv_meter_set_scale_ticks(meter, scale_min, 61, 2, 10,
                             lv_palette_main(LV_PALETTE_CYAN));
	lv_meter_set_scale_range(meter, scale_min, 0, 60, 360, 270);

	scale_hour = lv_meter_add_scale(meter);
	lv_meter_set_scale_ticks(meter, scale_hour, 12, 0, 0,
                             lv_palette_darken(LV_PALETTE_CYAN, 2));
	lv_meter_set_scale_major_ticks(meter, scale_hour, 1, 4, 20,
                                   lv_color_white(), -50);
	lv_meter_set_scale_range(meter, scale_hour, 1, 11, 330, 300);

	indic_min = lv_meter_add_needle_line(
		meter, scale_min, 4, lv_color_white(), -25);
	indic_hour = lv_meter_add_needle_line(
		meter, scale_hour, 8, lv_color_white(), -40);

    label = lv_label_create(app->screen);
    lv_label_set_text(label, "00:00");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -20);
}

extern struct tm extern_time;

void loop(struct app *app) {
    struct tm *tm = &extern_time;

    /* printk("time: %02d hour: %02d min\n", tm->tm_hour, tm->tm_min); */
    /* tm->tm_hour += 2; */
    /* tm->tm_hour %= 24; */
    
    lv_label_set_text_fmt(label, "%02d:%02d", tm->tm_hour, tm->tm_min);
    
    lv_meter_set_indicator_value(meter, indic_min,  tm->tm_min);
    lv_meter_set_indicator_value(meter, indic_hour, (tm->tm_hour + 11) % 12);

    k_sleep(K_MSEC(50));
}

void icon_create(struct app *app) {
    lv_obj_t *meter;
    lv_meter_scale_t *scale_min, *scale_hour;

    meter = lv_meter_create(app->icon);
    lv_obj_set_size(meter, 160, 160);
    lv_obj_center(meter);

    scale_min = lv_meter_add_scale(meter);
	lv_meter_set_scale_ticks(meter, scale_min, 61, 2, 10,
                             lv_palette_main(LV_PALETTE_CYAN));
	lv_meter_set_scale_range(meter, scale_min, 0, 60, 360, 270);

	scale_hour = lv_meter_add_scale(meter);
	lv_meter_set_scale_ticks(meter, scale_hour, 12, 0, 0,
                             lv_palette_darken(LV_PALETTE_CYAN, 2));
	lv_meter_set_scale_major_ticks(meter, scale_hour, 1, 4, 20,
                                   lv_color_white(), -1000);
	lv_meter_set_scale_range(meter, scale_hour, 1, 12, 330, 300);
}
