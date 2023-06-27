#define SUBSYS_NAME network
#include <subsys/base.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/wifi_mgmt.h>

struct net_mgmt_event_callback wifi_cb;
struct net_mgmt_event_callback ipv4_cb;

int wifi_connect(void);
int wifi_status(void);
int dhcp_start(void);

void handle_wifi_connect_result(struct net_mgmt_event_callback *cb) {
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (status->status) {
        LOG_ERR("connection failed: reconecting");
        wifi_connect();
    } else {
        LOG_INF("connected");
        wifi_status();
        dhcp_start();
    }    
}

void handle_wifi_disconnect_result(struct net_mgmt_event_callback *cb) {
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (!status->status) {
        LOG_INF("disconnected: reco");
        wifi_status();
    }    
}

void handle_ipv4_result(struct net_if *iface) {
    int i = 0;

    for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {

        char buf[NET_IPV4_ADDR_LEN];

        if (iface->config.ip.ipv4->unicast[i].addr_type != NET_ADDR_DHCP) {
            continue;
        }

        LOG_INF("IPv4 address: %s",
                net_addr_ntop(AF_INET,
                              &iface->config.ip.ipv4->unicast[i].address.in_addr,
                              buf, sizeof(buf)));
        LOG_INF("subnet: %s",
                net_addr_ntop(AF_INET,
                              &iface->config.ip.ipv4->netmask,
                              buf, sizeof(buf)));
        LOG_INF("router: %s",
                net_addr_ntop(AF_INET,
                              &iface->config.ip.ipv4->gw,
                              buf, sizeof(buf)));
    }
}

void wifi_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface) {
    switch (mgmt_event) {

    case NET_EVENT_WIFI_CONNECT_RESULT:
        handle_wifi_connect_result(cb);
        break;

    case NET_EVENT_WIFI_DISCONNECT_RESULT:
        handle_wifi_disconnect_result(cb);
        break;

    case NET_EVENT_IPV4_ADDR_ADD:
        handle_ipv4_result(iface);
        break;

    default:
        break;
    }
}

int wifi_connect(void) {
    struct net_if *iface = net_if_get_default();

    struct wifi_connect_req_params wifi_params = {0};
    wifi_params.ssid = "TP-Link_7224";
    wifi_params.psk  = "79412339";
    wifi_params.ssid_length = 12;
    wifi_params.psk_length  = 8;
    wifi_params.channel = WIFI_CHANNEL_ANY;
    wifi_params.security = WIFI_SECURITY_TYPE_PSK;
    wifi_params.band = WIFI_FREQ_BAND_2_4_GHZ; 
    wifi_params.mfp = WIFI_MFP_OPTIONAL;

    int err;
    err = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &wifi_params, sizeof(struct wifi_connect_req_params));
    if (err) {
        LOG_ERR("failed to connect to wifi: %d: %s",
                err, strerror(err));
        return err;
    }
    LOG_INF("wifi connected");

    return 0;
}

int wifi_status(void) {
    struct net_if *iface = net_if_get_default();

    struct wifi_iface_status status = {0};

    int err;
    err = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status,	sizeof(struct wifi_iface_status));
    if (err) {
        LOG_ERR("failed to get wifi status: %d: %s",
                err, strerror(err));
        return err;
    }

    if (status.state >= WIFI_STATE_ASSOCIATED) {
        LOG_INF("ssid: %-32s", status.ssid);
        LOG_INF("band: %s", wifi_band_txt(status.band));
        LOG_INF("channel: %d", status.channel);
        LOG_INF("security: %s", wifi_security_txt(status.security));
        LOG_INF("RSSI: %d", status.rssi);
    }
    
    LOG_INF("wifi status");

    return 0;
}

int dhcp_start(void) {
    struct net_if *iface = net_if_get_default();

    net_dhcpv4_start(iface);
    return 0;
}

int init(void) {
    net_mgmt_init_event_callback(
        &wifi_cb,
        wifi_event_handler,
        NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_init_event_callback(
        &ipv4_cb,
        wifi_event_handler,
        NET_EVENT_IPV4_ADDR_ADD);

    net_mgmt_add_event_callback(&wifi_cb);
    net_mgmt_add_event_callback(&ipv4_cb);

    wifi_connect();
}
