#ifndef SUBSYS_LIST_H_
#define SUBSYS_LIST_H_

#include <zephyr/kernel.h>

struct subsys {
    const char * name;
    struct k_thread *thread;
};

extern struct subsys *subsys_list;
extern size_t subsys_count;
extern size_t subsys_index;

#endif
