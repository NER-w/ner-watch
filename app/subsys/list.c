#include <subsys/list.h>

#include <zephyr/kernel.h>
#include <zephyr/init.h>

struct subsys *subsys_list = NULL;
size_t subsys_count = 0;
size_t subsys_index = 0;

int subsys_list_init(void) {
    subsys_list = k_malloc(subsys_count * sizeof(struct subsys));
    return subsys_list == NULL;
}

SYS_INIT(subsys_list_init, APPLICATION, 2);
