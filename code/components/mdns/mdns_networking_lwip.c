/*
 * SPDX-FileCopyrightText: 2022-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * MDNS Server Networking
 *
 */
#include <string.h>
#include "esp_log.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "lwip/igmp.h"
#include "lwip/udp.h"
#include "lwip/mld6.h"
#include "lwip/priv/tcpip_priv.h"
#include "esp_system.h"
#include "esp_event.h"
#include "mdns_networking.h"
#include "esp_netif_net_stack.h"
#include "mdns_mem_caps.h"

/*
 * MDNS Server Networking
 *
 */
enum interface_protocol {
    PROTO_IPV4 = 1 << MDNS_IP_PROTOCOL_V4,
    PROTO_IPV6 = 1 << MDNS_IP_PROTOCOL_V6
};

typedef struct interfaces {
    bool ready;
    int proto;
} interfaces_t;

static interfaces_t s_interfaces[MDNS_MAX_INTERFACES];

static struct udp_pcb *_pcb_main = NULL;

static const char *TAG = "mdns_networking";

static void _udp_recv(void *arg, struct udp_pcb *upcb, struct pbuf *pb, const ip_addr_t *raddr, uint16_t rport);

/**
 * @brief  Low level UDP PCB Initialize
 */
static esp_err_t _udp_pcb_main_init(void)
{
    if (_pcb_main) {
        return ESP_OK;
    }
    _pcb_main = udp_new();
    if (!_pcb_main) {
        return ESP_ERR_NO_MEM;
    }
    if (udp_bind(_pcb_main, IP_ANY_TYPE, MDNS_SERVICE_PORT) != 0) {
        udp_remove(_pcb_main);
        _pcb_main = NULL;
        return ESP_ERR_INVALID_STATE;
    }
    _pcb_main->mcast_ttl = 255;
    _pcb_main->remote_port = MDNS_SERVICE_PORT;
    ip_addr_copy(_pcb_main->remote_ip, *(IP_ANY_TYPE));
    udp_recv(_pcb_main, &_udp_recv, NULL);
    return ESP_OK;
}

/**
 * @brief  Low level UDP PCB Free
 */
static void _udp_pcb_main_deinit(void)
{
    if (_pcb_main) {
        udp_recv(_pcb_main, NULL, NULL);
        udp_disconnect(_pcb_main);
        udp_remove(_pcb_main);
        _pcb_main = NULL;
    }
}

/**
 * @brief  Low level UDP Multicast membership control
 */
static esp_err_t _udp_join_group(mdns_if_t if_inx, mdns_ip_protocol_t ip_protocol, bool join)
{
    struct netif *netif = NULL;
    esp_netif_t *tcpip_if = _mdns_get_esp_netif(if_inx);

    if (!esp_netif_is_netif_up(tcpip_if)) {
        // Network interface went down before event propagated, skipping IGMP config
        return ESP_ERR_INVALID_STATE;
    }

    netif = esp_netif_get_netif_impl(tcpip_if);
    assert(netif);

#if LWIP_IPV4
    if (ip_protocol == MDNS_IP_PROTOCOL_V4) {
        ip4_addr_t multicast_addr;
        IP4_ADDR(&multicast_addr, 224, 0, 0, 251);

        if (join) {
            if (igmp_joingroup_netif(netif, &multicast_addr)) {
                return ESP_ERR_INVALID_STATE;
            }
        } else {
            if (igmp_leavegroup_netif(netif, &multicast_addr)) {
                return ESP_ERR_INVALID_STATE;
            }
        }
    }
#endif // LWIP_IPV4
#if LWIP_IPV6
    if (ip_protocol == MDNS_IP_PROTOCOL_V6) {
        ip_addr_t multicast_addr = IPADDR6_INIT(0x000002ff, 0, 0, 0xfb000000);

        if (join) {
            if (mld6_joingroup_netif(netif, ip_2_ip6(&multicast_addr))) {
                return ESP_ERR_INVALID_STATE;
            }
        } else {
            if (mld6_leavegroup_netif(netif, ip_2_ip6(&multicast_addr))) {
                return ESP_ERR_INVALID_STATE;
            }
        }
    }
#endif // LWIP_IPV6
    return ESP_OK;
}

/**
 * @brief  the receive callback of the raw udp api. Packets are received here
 *
 */
static void _udp_recv(void *arg, struct udp_pcb *upcb, struct pbuf *pb, const ip_addr_t *raddr, uint16_t rport)
{

    uint8_t i;
    while (pb != NULL) {
        struct pbuf *this_pb = pb;
        pb = pb->next;
        this_pb->next = NULL;

        mdns_rx_packet_t *packet = (mdns_rx_packet_t *)mdns_mem_malloc(sizeof(mdns_rx_packet_t));
        if (!packet) {
            HOOK_MALLOC_FAILED;
            //missed packet - no memory
            pbuf_free(this_pb);
            continue;
        }

        packet->tcpip_if = MDNS_MAX_INTERFACES;
        packet->pb = this_pb;
        packet->src_port = rport;
#if LWIP_IPV4 && LWIP_IPV6
        packet->src.type = raddr->type;
        memcpy(&packet->src.u_addr, &raddr->u_addr, sizeof(raddr->u_addr));
#elif LWIP_IPV4
        packet->src.type = IPADDR_TYPE_V4;
        packet->src.u_addr.ip4.addr = raddr->addr;
#elif LWIP_IPV6
        packet->src.type = IPADDR_TYPE_V6;
        memcpy(&packet->src.u_addr.ip6, raddr, sizeof(ip_addr_t));
#endif
        packet->dest.type = packet->src.type;

#if LWIP_IPV4
        if (packet->src.type == IPADDR_TYPE_V4) {
            packet->ip_protocol = MDNS_IP_PROTOCOL_V4;
            struct ip_hdr *iphdr = (struct ip_hdr *)(((uint8_t *)(packet->pb->payload)) - UDP_HLEN - IP_HLEN);
            packet->dest.u_addr.ip4.addr = iphdr->dest.addr;
            packet->multicast = ip4_addr_ismulticast(&(packet->dest.u_addr.ip4));
        }
#endif // LWIP_IPV4
#if LWIP_IPV6
        if (packet->src.type == IPADDR_TYPE_V6) {
            packet->ip_protocol = MDNS_IP_PROTOCOL_V6;
            struct ip6_hdr *ip6hdr = (struct ip6_hdr *)(((uint8_t *)(packet->pb->payload)) - UDP_HLEN - IP6_HLEN);
            memcpy(&packet->dest.u_addr.ip6.addr, (uint8_t *)ip6hdr->dest.addr, 16);
            packet->multicast = ip6_addr_ismulticast(&(packet->dest.u_addr.ip6));
        }
#endif // LWIP_IPV6

        //lwip does not return the proper pcb if you have more than one for the same multicast address (but different interfaces)
        struct netif *netif = NULL;
        bool found = false;
        for (i = 0; i < MDNS_MAX_INTERFACES; i++) {
            netif = esp_netif_get_netif_impl(_mdns_get_esp_netif(i));
            if (s_interfaces[i].proto && netif && netif == ip_current_input_netif()) {
#if LWIP_IPV4
                if (packet->src.type == IPADDR_TYPE_V4) {
                    if ((packet->src.u_addr.ip4.addr & ip_2_ip4(&netif->netmask)->addr) != (ip_2_ip4(&netif->ip_addr)->addr & ip_2_ip4(&netif->netmask)->addr)) {
                        //packet source is not in the same subnet
                        break;
                    }
                }
#endif // LWIP_IPV4
                packet->tcpip_if = i;
                found = true;
                break;
            }
        }

        if (!found || _mdns_send_rx_action(packet) != ESP_OK) {
            pbuf_free(this_pb);
            mdns_mem_free(packet);
        }
    }

}

bool mdns_is_netif_ready(mdns_if_t netif, mdns_ip_protocol_t ip_proto)
{
    return s_interfaces[netif].ready &&
           s_interfaces[netif].proto & (ip_proto == MDNS_IP_PROTOCOL_V4 ? PROTO_IPV4 : PROTO_IPV6);
}

/**
 * @brief  Check if any of the interfaces is up
 */
static bool _udp_pcb_is_in_use(void)
{
    int i, p;
    for (i = 0; i < MDNS_MAX_INTERFACES; i++) {
        for (p = 0; p < MDNS_IP_PROTOCOL_MAX; p++) {
            if (mdns_is_netif_ready(i, p)) {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief  Stop PCB Main code
 */
static void _udp_pcb_deinit(mdns_if_t tcpip_if, mdns_ip_protocol_t ip_protocol)
{
    s_interfaces[tcpip_if].proto &= ~(ip_protocol == MDNS_IP_PROTOCOL_V4 ? PROTO_IPV4 : PROTO_IPV6);
    if (s_interfaces[tcpip_if].proto == 0) {
        s_interfaces[tcpip_if].ready = false;
        _udp_join_group(tcpip_if, ip_protocol, false);
        if (!_udp_pcb_is_in_use()) {
            _udp_pcb_main_deinit();
        }
    }
}

/**
 * @brief  Start PCB Main code
 */
static esp_err_t _udp_pcb_init(mdns_if_t tcpip_if, mdns_ip_protocol_t ip_protocol)
{
    if (mdns_is_netif_ready(tcpip_if, ip_protocol)) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = _udp_join_group(tcpip_if, ip_protocol, true);
    if (err) {
        return err;
    }

    err = _udp_pcb_main_init();
    if (err) {
        return err;
    }
    s_interfaces[tcpip_if].proto |= (ip_protocol == MDNS_IP_PROTOCOL_V4 ? PROTO_IPV4 : PROTO_IPV6);
    s_interfaces[tcpip_if].ready = true;

    return ESP_OK;
}

typedef struct {
    struct tcpip_api_call_data call;
    mdns_if_t tcpip_if;
    mdns_ip_protocol_t ip_protocol;
    struct pbuf *pbt;
    const ip_addr_t *ip;
    uint16_t port;
    esp_err_t err;
} mdns_api_call_t;

/**
 * @brief  Start PCB from LwIP thread
 */
static err_t _mdns_pcb_init_api(struct tcpip_api_call_data *api_call_msg)
{
    mdns_api_call_t *msg = (mdns_api_call_t *)api_call_msg;
    msg->err = _udp_pcb_init(msg->tcpip_if, msg->ip_protocol) == ESP_OK ? ERR_OK : ERR_IF;
    return msg->err;
}

/**
 * @brief  Stop PCB from LwIP thread
 */
static err_t _mdns_pcb_deinit_api(struct tcpip_api_call_data *api_call_msg)
{
    mdns_api_call_t *msg = (mdns_api_call_t *)api_call_msg;
    _udp_pcb_deinit(msg->tcpip_if, msg->ip_protocol);
    msg->err = ESP_OK;
    return ESP_OK;
}

/*
 * Non-static functions below are
 *  - _mdns prefixed
 *  - commented in mdns_networking.h header
 */
esp_err_t _mdns_pcb_init(mdns_if_t tcpip_if, mdns_ip_protocol_t ip_protocol)
{
    mdns_api_call_t msg = {
        .tcpip_if = tcpip_if,
        .ip_protocol = ip_protocol
    };
    tcpip_api_call(_mdns_pcb_init_api, &msg.call);
    return msg.err;
}

esp_err_t _mdns_pcb_deinit(mdns_if_t tcpip_if, mdns_ip_protocol_t ip_protocol)
{
    mdns_api_call_t msg = {
        .tcpip_if = tcpip_if,
        .ip_protocol = ip_protocol
    };
    tcpip_api_call(_mdns_pcb_deinit_api, &msg.call);
    return msg.err;
}

static err_t _mdns_udp_pcb_write_api(struct tcpip_api_call_data *api_call_msg)
{
    void *nif = NULL;
    mdns_api_call_t *msg = (mdns_api_call_t *)api_call_msg;
    nif = esp_netif_get_netif_impl(_mdns_get_esp_netif(msg->tcpip_if));
    if (!nif || !mdns_is_netif_ready(msg->tcpip_if, msg->ip_protocol) || _pcb_main == NULL) {
        pbuf_free(msg->pbt);
        msg->err = ERR_IF;
        return ERR_IF;
    }
    esp_err_t err = udp_sendto_if(_pcb_main, msg->pbt, msg->ip, msg->port, (struct netif *)nif);
    pbuf_free(msg->pbt);
    msg->err = err;
    return err;
}

size_t _mdns_udp_pcb_write(mdns_if_t tcpip_if, mdns_ip_protocol_t ip_protocol, const esp_ip_addr_t *ip, uint16_t port, uint8_t *data, size_t len)
{
    struct pbuf *pbt = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (pbt == NULL) {
        return 0;
    }
    memcpy((uint8_t *)pbt->payload, data, len);

    ip_addr_t ip_add_copy;
#if LWIP_IPV6 && LWIP_IPV4
    ip_add_copy.type = ip->type;
    memcpy(&(ip_add_copy.u_addr), &(ip->u_addr), sizeof(ip_add_copy.u_addr));
#elif LWIP_IPV4
    ip_add_copy.addr = ip->u_addr.ip4.addr;
#elif LWIP_IPV6
#if LWIP_IPV6_SCOPES
    ip_add_copy.zone = ip->u_addr.ip6.zone;
#endif // LWIP_IPV6_SCOPES
    memcpy(ip_add_copy.addr, ip->u_addr.ip6.addr, sizeof(ip_add_copy.addr));
#endif

    mdns_api_call_t msg = {
        .tcpip_if = tcpip_if,
        .ip_protocol = ip_protocol,
        .pbt = pbt,
        .ip = &ip_add_copy,
        .port = port
    };
    tcpip_api_call(_mdns_udp_pcb_write_api, &msg.call);

    if (msg.err) {
        return 0;
    }
    return len;
}

void *_mdns_get_packet_data(mdns_rx_packet_t *packet)
{
    return packet->pb->payload;
}

size_t _mdns_get_packet_len(mdns_rx_packet_t *packet)
{
    return packet->pb->len;
}

void _mdns_packet_free(mdns_rx_packet_t *packet)
{
    pbuf_free(packet->pb);
    mdns_mem_free(packet);
}
