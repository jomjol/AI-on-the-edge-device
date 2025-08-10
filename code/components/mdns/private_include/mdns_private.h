/*
 * SPDX-FileCopyrightText: 2015-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef MDNS_PRIVATE_H_
#define MDNS_PRIVATE_H_

#include "sdkconfig.h"
#include "mdns.h"
#include "esp_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_system.h"

#ifdef CONFIG_MDNS_ENABLE_DEBUG_PRINTS
#define MDNS_ENABLE_DEBUG
#define _mdns_dbg_printf(...) printf(__VA_ARGS__)
#endif

/** Number of predefined interfaces */
#ifndef CONFIG_MDNS_PREDEF_NETIF_STA
#define CONFIG_MDNS_PREDEF_NETIF_STA 0
#endif
#ifndef CONFIG_MDNS_PREDEF_NETIF_AP
#define CONFIG_MDNS_PREDEF_NETIF_AP 0
#endif
#ifndef CONFIG_MDNS_PREDEF_NETIF_ETH
#define CONFIG_MDNS_PREDEF_NETIF_ETH 0
#endif
#define MDNS_MAX_PREDEF_INTERFACES (CONFIG_MDNS_PREDEF_NETIF_STA + CONFIG_MDNS_PREDEF_NETIF_AP + CONFIG_MDNS_PREDEF_NETIF_ETH)

#ifdef CONFIG_LWIP_IPV6_NUM_ADDRESSES
#define NETIF_IPV6_MAX_NUMS CONFIG_LWIP_IPV6_NUM_ADDRESSES
#else
#define NETIF_IPV6_MAX_NUMS 3
#endif

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 1, 0)
/* CONFIG_LWIP_IPV4 was introduced in IDF v5.1 */
/* For IDF v5.0, set CONFIG_LWIP_IPV4 to 1 by default */
#ifndef CONFIG_LWIP_IPV4
#define CONFIG_LWIP_IPV4 1
#endif // CONFIG_LWIP_IPV4
#endif // ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 1, 0)

/** Number of configured interfaces */
#if MDNS_MAX_PREDEF_INTERFACES > CONFIG_MDNS_MAX_INTERFACES
#warning Number of configured interfaces is less then number of predefined interfaces. Please update CONFIG_MDNS_MAX_INTERFACES.
#define MDNS_MAX_INTERFACES (MDNS_MAX_PREDEF_INTERFACES)
#else
#define MDNS_MAX_INTERFACES (CONFIG_MDNS_MAX_INTERFACES)
#endif

/** The maximum number of services */
#define MDNS_MAX_SERVICES           CONFIG_MDNS_MAX_SERVICES

#define MDNS_ANSWER_PTR_TTL         4500
#define MDNS_ANSWER_TXT_TTL         4500
#define MDNS_ANSWER_SRV_TTL         120
#define MDNS_ANSWER_A_TTL           120
#define MDNS_ANSWER_AAAA_TTL        120

#define MDNS_FLAGS_QUERY_REPSONSE   0x8000
#define MDNS_FLAGS_AUTHORITATIVE    0x0400
#define MDNS_FLAGS_QR_AUTHORITATIVE (MDNS_FLAGS_QUERY_REPSONSE | MDNS_FLAGS_AUTHORITATIVE)
#define MDNS_FLAGS_DISTRIBUTED      0x0200

#define MDNS_NAME_REF               0xC000

//custom type! only used by this implementation
//to help manage service discovery handling
#define MDNS_TYPE_SDPTR             0x0032

#define MDNS_CLASS_IN               0x0001
#define MDNS_CLASS_ANY              0x00FF
#define MDNS_CLASS_IN_FLUSH_CACHE   0x8001

#define MDNS_ANSWER_ALL             0x3F
#define MDNS_ANSWER_PTR             0x08
#define MDNS_ANSWER_TXT             0x04
#define MDNS_ANSWER_SRV             0x02
#define MDNS_ANSWER_A               0x01
#define MDNS_ANSWER_AAAA            0x10
#define MDNS_ANSWER_NSEC            0x20
#define MDNS_ANSWER_SDPTR           0x80
#define MDNS_ANSWER_AAAA_SIZE       16

#define MDNS_SERVICE_PORT           5353                    // UDP port that the server runs on
#define MDNS_SERVICE_STACK_DEPTH    CONFIG_MDNS_TASK_STACK_SIZE
#define MDNS_TASK_PRIORITY          CONFIG_MDNS_TASK_PRIORITY
#if (MDNS_TASK_PRIORITY > ESP_TASK_PRIO_MAX)
#error "mDNS task priority is higher than ESP_TASK_PRIO_MAX"
#elif (MDNS_TASK_PRIORITY > ESP_TASKD_EVENT_PRIO)
#warning "mDNS task priority is higher than ESP_TASKD_EVENT_PRIO, mDNS library might not work correctly"
#endif
#define MDNS_TASK_AFFINITY          CONFIG_MDNS_TASK_AFFINITY
#define MDNS_SERVICE_ADD_TIMEOUT_MS CONFIG_MDNS_SERVICE_ADD_TIMEOUT_MS

#define MDNS_PACKET_QUEUE_LEN       16                      // Maximum packets that can be queued for parsing
#define MDNS_ACTION_QUEUE_LEN       CONFIG_MDNS_ACTION_QUEUE_LEN  // Maximum actions pending to the server
#define MDNS_TXT_MAX_LEN            1024                    // Maximum string length of text data in TXT record
#define MDNS_MAX_PACKET_SIZE        1460                    // Maximum size of mDNS  outgoing packet

#define MDNS_HEAD_LEN               12
#define MDNS_HEAD_ID_OFFSET         0
#define MDNS_HEAD_FLAGS_OFFSET      2
#define MDNS_HEAD_QUESTIONS_OFFSET  4
#define MDNS_HEAD_ANSWERS_OFFSET    6
#define MDNS_HEAD_SERVERS_OFFSET    8
#define MDNS_HEAD_ADDITIONAL_OFFSET 10

#define MDNS_TYPE_OFFSET            0
#define MDNS_CLASS_OFFSET           2
#define MDNS_TTL_OFFSET             4
#define MDNS_LEN_OFFSET             8
#define MDNS_DATA_OFFSET            10

#define MDNS_SRV_PRIORITY_OFFSET    0
#define MDNS_SRV_WEIGHT_OFFSET      2
#define MDNS_SRV_PORT_OFFSET        4
#define MDNS_SRV_FQDN_OFFSET        6

#define MDNS_TIMER_PERIOD_US        (CONFIG_MDNS_TIMER_PERIOD_MS*1000)

#define MDNS_SERVICE_LOCK()     xSemaphoreTake(_mdns_service_semaphore, portMAX_DELAY)
#define MDNS_SERVICE_UNLOCK()   xSemaphoreGive(_mdns_service_semaphore)

#define queueToEnd(type, queue, item)       \
    if (!queue) {                           \
        queue = item;                       \
    } else {                                \
        type * _q = queue;                  \
        while (_q->next) { _q = _q->next; } \
        _q->next = item;                    \
    }

#define queueDetach(type, queue, item)              \
    if (queue) {                                    \
        if (queue == item) {                        \
            queue = queue->next;                    \
        } else {                                    \
            type * _q = queue;                      \
            while (_q->next && _q->next != item) {  \
                _q = _q->next;                      \
            }                                       \
            if (_q->next == item) {                 \
                _q->next = item->next;              \
                item->next = NULL;                  \
            }                                       \
        }                                           \
    }

#define queueFree(type, queue)  while (queue) { type * _q = queue; queue = queue->next; free(_q); }

#define PCB_STATE_IS_PROBING(s) (s->state > PCB_OFF && s->state < PCB_ANNOUNCE_1)
#define PCB_STATE_IS_ANNOUNCING(s) (s->state > PCB_PROBE_3 && s->state < PCB_RUNNING)
#define PCB_STATE_IS_RUNNING(s) (s->state == PCB_RUNNING)

#ifndef HOOK_MALLOC_FAILED
#define HOOK_MALLOC_FAILED  ESP_LOGE(TAG, "Cannot allocate memory (line: %d, free heap: %" PRIu32 " bytes)", __LINE__, esp_get_free_heap_size());
#endif

typedef size_t mdns_if_t;

typedef enum {
    PCB_OFF, PCB_DUP, PCB_INIT,
    PCB_PROBE_1, PCB_PROBE_2, PCB_PROBE_3,
    PCB_ANNOUNCE_1, PCB_ANNOUNCE_2, PCB_ANNOUNCE_3,
    PCB_RUNNING
} mdns_pcb_state_t;

typedef enum {
    MDNS_ANSWER, MDNS_NS, MDNS_EXTRA
} mdns_parsed_record_type_t;

typedef enum {
    ACTION_SYSTEM_EVENT,
    ACTION_HOSTNAME_SET,
    ACTION_INSTANCE_SET,
    ACTION_SEARCH_ADD,
    ACTION_SEARCH_SEND,
    ACTION_SEARCH_END,
    ACTION_BROWSE_ADD,
    ACTION_BROWSE_SYNC,
    ACTION_BROWSE_END,
    ACTION_TX_HANDLE,
    ACTION_RX_HANDLE,
    ACTION_TASK_STOP,
    ACTION_DELEGATE_HOSTNAME_ADD,
    ACTION_DELEGATE_HOSTNAME_REMOVE,
    ACTION_DELEGATE_HOSTNAME_SET_ADDR,
    ACTION_MAX
} mdns_action_type_t;


typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t questions; //QDCOUNT
    uint16_t answers;   //ANCOUNT
    uint16_t servers;   //NSCOUNT
    uint16_t additional;//ARCOUNT
} mdns_header_t;

typedef struct {
    char host[MDNS_NAME_BUF_LEN]; // hostname for A/AAAA records, instance name for SRV records
    char service[MDNS_NAME_BUF_LEN];
    char proto[MDNS_NAME_BUF_LEN];
    char domain[MDNS_NAME_BUF_LEN];
    uint8_t parts;
    uint8_t sub;
    bool    invalid;
} mdns_name_t;

typedef struct mdns_parsed_question_s {
    struct mdns_parsed_question_s *next;
    uint16_t type;
    bool sub;
    bool unicast;
    char *host;
    char *service;
    char *proto;
    char *domain;
} mdns_parsed_question_t;

typedef struct mdns_parsed_record_s {
    struct mdns_parsed_record_s *next;
    mdns_parsed_record_type_t record_type;
    uint16_t type;
    uint16_t clas;
    uint8_t flush;
    uint32_t ttl;
    char *host;
    char *service;
    char *proto;
    char *domain;
    uint16_t data_len;
    uint8_t *data;
} mdns_parsed_record_t;

typedef struct {
    mdns_if_t tcpip_if;
    mdns_ip_protocol_t ip_protocol;
    esp_ip_addr_t src;
    uint16_t src_port;
    uint8_t multicast;
    uint8_t authoritative;
    uint8_t probe;
    uint8_t discovery;
    uint8_t distributed;
    mdns_parsed_question_t *questions;
    mdns_parsed_record_t *records;
    uint16_t id;
} mdns_parsed_packet_t;

typedef struct {
    mdns_if_t tcpip_if;
    mdns_ip_protocol_t ip_protocol;
    struct pbuf *pb;
    esp_ip_addr_t src;
    esp_ip_addr_t dest;
    uint16_t src_port;
    uint8_t multicast;
} mdns_rx_packet_t;

typedef struct mdns_txt_linked_item_s {
    const char *key;                        /*!< item key name */
    char *value;                            /*!< item value string */
    uint8_t value_len;                      /*!< item value length */
    struct mdns_txt_linked_item_s *next;    /*!< next result, or NULL for the last result in the list */
} mdns_txt_linked_item_t;

typedef struct mdns_subtype_s {
    const char *subtype;                    /*!< subtype */
    struct mdns_subtype_s *next;            /*!< next result, or NULL for the last result in the list */
} mdns_subtype_t;

typedef struct {
    const char *instance;
    const char *service;
    const char *proto;
    const char *hostname;
    uint16_t priority;
    uint16_t weight;
    uint16_t port;
    mdns_txt_linked_item_t *txt;
    mdns_subtype_t *subtype;
} mdns_service_t;

typedef struct mdns_srv_item_s {
    struct mdns_srv_item_s *next;
    mdns_service_t *service;
} mdns_srv_item_t;

typedef struct mdns_out_question_s {
    struct mdns_out_question_s *next;
    uint16_t type;
    bool unicast;
    const char *host;
    const char *service;
    const char *proto;
    const char *domain;
    bool own_dynamic_memory;
} mdns_out_question_t;

typedef struct mdns_host_item_t {
    const char *hostname;
    mdns_ip_addr_t *address_list;
    struct mdns_host_item_t *next;
} mdns_host_item_t;

typedef struct mdns_out_answer_s {
    struct mdns_out_answer_s *next;
    uint16_t type;
    uint8_t bye;
    uint8_t flush;
    mdns_service_t *service;
    mdns_host_item_t *host;
    const char *custom_instance;
    const char *custom_service;
    const char *custom_proto;
} mdns_out_answer_t;

typedef struct mdns_tx_packet_s {
    struct mdns_tx_packet_s *next;
    uint32_t send_at;
    mdns_if_t tcpip_if;
    mdns_ip_protocol_t ip_protocol;
    esp_ip_addr_t dst;
    uint16_t port;
    uint16_t flags;
    uint8_t distributed;
    mdns_out_question_t *questions;
    mdns_out_answer_t *answers;
    mdns_out_answer_t *servers;
    mdns_out_answer_t *additional;
    bool queued;
    uint16_t id;
} mdns_tx_packet_t;

typedef struct {
    mdns_pcb_state_t state;
    mdns_srv_item_t **probe_services;
    uint8_t probe_services_len;
    uint8_t probe_ip;
    uint8_t probe_running;
    uint16_t failed_probes;
} mdns_pcb_t;

typedef enum {
    SEARCH_OFF,
    SEARCH_INIT,
    SEARCH_RUNNING,
    SEARCH_MAX
} mdns_search_once_state_t;

typedef enum {
    BROWSE_OFF,
    BROWSE_INIT,
    BROWSE_RUNNING,
    BROWSE_MAX
} mdns_browse_state_t;

typedef struct mdns_search_once_s {
    struct mdns_search_once_s *next;

    mdns_search_once_state_t state;
    uint32_t started_at;
    uint32_t sent_at;
    uint32_t timeout;
    mdns_query_notify_t notifier;
    SemaphoreHandle_t done_semaphore;
    uint16_t type;
    bool unicast;
    uint8_t max_results;
    uint8_t num_results;
    char *instance;
    char *service;
    char *proto;
    mdns_result_t *result;
} mdns_search_once_t;

typedef struct mdns_browse_s {
    struct mdns_browse_s *next;

    mdns_browse_state_t state;
    mdns_browse_notify_t notifier;

    char *service;
    char *proto;
    mdns_result_t *result;
} mdns_browse_t;

typedef struct mdns_browse_result_sync_t {
    mdns_result_t *result;
    struct mdns_browse_result_sync_t *next;
} mdns_browse_result_sync_t;

typedef struct mdns_browse_sync {
    mdns_browse_t *browse;
    mdns_browse_result_sync_t *sync_result;
} mdns_browse_sync_t;

typedef struct mdns_server_s {
    struct {
        mdns_pcb_t pcbs[MDNS_IP_PROTOCOL_MAX];
    } interfaces[MDNS_MAX_INTERFACES];
    const char *hostname;
    const char *instance;
    mdns_srv_item_t *services;
    QueueHandle_t action_queue;
    SemaphoreHandle_t action_sema;
    mdns_tx_packet_t *tx_queue_head;
    mdns_search_once_t *search_once;
    esp_timer_handle_t timer_handle;
    mdns_browse_t *browse;
} mdns_server_t;

typedef struct {
    mdns_action_type_t type;
    union {
        struct {
            char *hostname;
        } hostname_set;
        char *instance;
        struct {
            mdns_if_t interface;
            mdns_event_actions_t event_action;
        } sys_event;
        struct {
            mdns_search_once_t *search;
        } search_add;
        struct {
            mdns_tx_packet_t *packet;
        } tx_handle;
        struct {
            mdns_rx_packet_t *packet;
        } rx_handle;
        struct {
            const char *hostname;
            mdns_ip_addr_t *address_list;
        } delegate_hostname;
        struct {
            mdns_browse_t *browse;
        } browse_add;
        struct {
            mdns_browse_sync_t *browse_sync;
        } browse_sync;
    } data;
} mdns_action_t;

/*
 * @brief  Convert mnds if to esp-netif handle
 *
 * @param  tcpip_if     mdns supported interface as internal enum
 *
 * @return
 *     - ptr to esp-netif on success
 *     - NULL if no available netif for current interface index
 */
esp_netif_t *_mdns_get_esp_netif(mdns_if_t tcpip_if);


#endif /* MDNS_PRIVATE_H_ */
