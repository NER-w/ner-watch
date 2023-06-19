#include "app/list.h"

#include <zephyr/kernel.h>
#include <zephyr/init.h>

struct app *app_list = NULL;
size_t app_count = 0;
size_t app_index = 0;

int app_list_init(void) {
    app_list = k_malloc(app_count * sizeof(struct app));
    return app_list == NULL;
}

SYS_INIT(app_list_init, APPLICATION, 2);

