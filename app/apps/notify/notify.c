#define APP_NAME notify
#include "app/base.h"

#include <zephyr/net/net_if.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/http/client.h>

static int sock;
struct back_work {
    struct k_work work;

    int sock;
} back_work;

void response_cb(struct http_response *rsp, enum http_final_call final_data, void *user_data) {
    ARG_UNUSED(user_data);

    switch (final_data) {
    case HTTP_DATA_MORE:
        LOG_INF("partial data received: %d", rsp->data_len);
        break;
    case HTTP_DATA_FINAL:
        LOG_INF("final data received: %d", rsp->data_len);
        break;
    default:
        break;
    }
    LOG_INF("response status code: %s", rsp->http_status);
}

void update_list(struct k_work *work) {
    struct back_work *back_work =
        CONTAINER_OF(work, struct back_work, work);

    struct http_request req = {0};

    req.method   = HTTP_GET;
    req.url      = "/appointment/1";
    req.host     = "1e04-2001-1c06-1b15-4700-8494-a060-34a4-9df1.ngrok-free.app";
    req.protocol = "HTTP/1.1";
    req.response = response_cb;

    int ret = http_client_req(back_work->sock, &req, 5000, NULL);
    
}

void setup(struct app *app) {
    printk("Hello from notify\n");

    lv_obj_t *arc;
    arc = lv_arc_create(app->screen);
    lv_obj_set_size(arc, 240, 240);

    lv_arc_set_rotation(arc, 180);
    lv_arc_set_bg_angles(arc, 120, 240);
    lv_arc_set_value(arc, 40);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_center(arc);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        LOG_ERR("failed to create socket");
    } else {            
        back_work.sock = sock;
        k_work_init(&back_work.work, update_list);
    }
}

void loop(struct app *app) {
    if (sock < 0) {
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0) {
            LOG_ERR("failed to create socket");
        } else {            
            back_work.sock = sock;
            k_work_init(&back_work.work, update_list);
            k_work_submit(&back_work.work);            
        }
    } else {
        k_work_submit(&back_work.work);
    }
    k_sleep(K_MSEC(30));
}

void icon_create(struct app *app) {
    lv_obj_t *label;

    label = lv_label_create(app->icon);
    lv_label_set_text(label, "Notify");
    lv_obj_center(label);
}
