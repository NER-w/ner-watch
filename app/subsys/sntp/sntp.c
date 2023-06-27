#define SUBSYS_NAME ntp
#define SUBSYS_STACK_SIZE 2560
#include <subsys/base.h>

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/sntp.h>

#include <time.h>
#include <string.h>

K_THREAD_STACK_DEFINE(ntp_requester_stack_area, 1024);
static struct k_thread ntp_requester_data;

struct tm extern_time;

/* const char *sntp_server_list[] = { */
/*     "pool.ntp.org", */
/*     "ntp.time.nl", */
/*     "time.cloudflare.com", */
/* }; */

/* static inline int sntp_attempt(const char *host, int32_t timeout, struct sntp_time *tm) { */
/*     int i, err; */
       
/*     for (i = 0, err = -1; i < 3 && err < 0; i++) */
/*         err = sntp_simple(host, timeout, tm); */

/*     return err; */
/* } */

void requester(void *arg1, void *arg2, void *arg3) {
    ARG_UNUSED(arg1); ARG_UNUSED(arg2); ARG_UNUSED(arg3);
    
    struct sntp_time timer = {0};
    struct tm        unix_time;

    int err;
    while (-42) {

        /* err = -1; */
        /* for (size_t i = 0; i < sizeof(sntp_server_list) && err < 0; i++) */
        /*     err = sntp_attempt(sntp_server_list[i], 30000, &timer); */

        err = sntp_simple("time.windows.com", 30000, &timer);
        if (err < 0) err = sntp_simple("pool.ntp.org", 30000, &timer);
        if (err < 0) err = sntp_simple("pool.ntp.org", 30000, &timer);
        
        if (err < 0) {
            LOG_ERR("failed to get sntp time: %d: %s",
                    err, strerror(-err));
        } else {

            gmtime_r(&timer.seconds, &unix_time);
            LOG_INF("unix epoch: %llu", timer.seconds);

            LOG_INF(
                "sntp time: 20%02d-%02d-%02d %02d:%02d:%02d",
                unix_time.tm_year, unix_time.tm_mon, unix_time.tm_mday,
                unix_time.tm_hour, unix_time.tm_min, unix_time.tm_sec);

            extern_time = unix_time;

        }
        
        k_sleep(K_SECONDS(30));
    }
}

int init(void) {

    k_thread_create(
        &ntp_requester_data, ntp_requester_stack_area,
        K_THREAD_STACK_SIZEOF(ntp_requester_stack_area),
        requester, NULL, NULL, NULL,
        7, 0, K_FOREVER);

    k_thread_start(&ntp_requester_data);
    return 0;
}
