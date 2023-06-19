#define APP_NAME clock
#define APP_STACK_SIZE 10240
#include "app/base.h"

lv_obj_t *meter;
lv_meter_scale_t *scale_min;
lv_meter_scale_t *scale_hour;
lv_meter_indicator_t *indic_min;
lv_meter_indicator_t *indic_hour;

void setup(struct app *app) {
    printk("Hello from clock\n");

    meter= lv_meter_create(app->screen);
    lv_obj_set_size(meter, 240, 240);
    lv_obj_center(meter);

    scale_min = lv_meter_add_scale(meter);
	lv_meter_set_scale_ticks(meter, scale_min, 61, 2, 10,
                             lv_palette_main(LV_PALETTE_CYAN));
	lv_meter_set_scale_range(meter, scale_min, 0, 60, 360, 270);

	scale_hour = lv_meter_add_scale(meter);
	lv_meter_set_scale_ticks(meter, scale_hour, 12, 0, 0,
                             lv_palette_darken(LV_PALETTE_CYAN, 2));
	lv_meter_set_scale_major_ticks(meter, scale_hour, 1, 4, 20,
                                   lv_color_white(), -50);
	lv_meter_set_scale_range(meter, scale_hour, 1, 12, 330, 300);

	indic_min = lv_meter_add_needle_line(
		meter, scale_min, 4, lv_color_white(), -25);
	indic_hour = lv_meter_add_needle_line(
		meter, scale_hour, 8, lv_color_white(), -40);

}

void loop(struct app *app) {
    static int count = 0;
    lv_meter_set_indicator_value(meter, indic_min, count % 60);
    lv_meter_set_indicator_value(meter, indic_hour, (count / 60 + 1) % 12);
    count++;
    k_sleep(K_MSEC(10));
}
