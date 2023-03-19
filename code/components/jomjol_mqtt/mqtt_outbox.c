/* This is a modification of https://github.com/espressif/esp-mqtt/blob/master/lib/mqtt_outbox.c
 * to use the PSRAM instead of the internal heap.
*/
#include "mqtt_outbox.h"
#include <stdlib.h>
#include <string.h>
#include "sys/queue.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

/* Enable this to use the PSRAM for MQTT Publishing.
 * This saves 10 kBytes of RAM, see https://github.com/jomjol/AI-on-the-edge-device/pull/2113
 * However we can run into PSRAM fragmentation issues, leading to insufficient large blocks to load the model.
 * See https://github.com/jomjol/AI-on-the-edge-device/issues/2200 */
#define USE_PSRAM

#ifdef CONFIG_MQTT_CUSTOM_OUTBOX
static const char *TAG = "outbox";

typedef struct outbox_item {
    char *buffer;
    int len;
    int msg_id;
    int msg_type;
    int msg_qos;
    outbox_tick_t tick;
    pending_state_t pending;
    STAILQ_ENTRY(outbox_item) next;
} outbox_item_t;

STAILQ_HEAD(outbox_list_t, outbox_item);


outbox_handle_t outbox_init(void)
{
#ifdef USE_PSRAM
    outbox_handle_t outbox = heap_caps_calloc(1, sizeof(struct outbox_list_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
#else
    outbox_handle_t outbox = calloc(1, sizeof(struct outbox_list_t));
#endif
    //ESP_MEM_CHECK(TAG, outbox, return NULL);
    STAILQ_INIT(outbox);
    return outbox;
}

outbox_item_handle_t outbox_enqueue(outbox_handle_t outbox, outbox_message_handle_t message, outbox_tick_t tick)
{
#ifdef USE_PSRAM
    outbox_item_handle_t item = heap_caps_calloc(1, sizeof(outbox_item_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
#else
    outbox_item_handle_t item = calloc(1, sizeof(outbox_item_t));
#endif
    //ESP_MEM_CHECK(TAG, item, return NULL);
    item->msg_id = message->msg_id;
    item->msg_type = message->msg_type;
    item->msg_qos = message->msg_qos;
    item->tick = tick;
    item->len =  message->len + message->remaining_len;
    item->pending = QUEUED;
#ifdef USE_PSRAM
    item->buffer = heap_caps_malloc(message->len + message->remaining_len, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
#else
    item->buffer = malloc(message->len + message->remaining_len);
#endif
    /*ESP_MEM_CHECK(TAG, item->buffer, {
        free(item);
        return NULL;
    });*/
    memcpy(item->buffer, message->data, message->len);
    if (message->remaining_data) {
        memcpy(item->buffer + message->len, message->remaining_data, message->remaining_len);
    }
    STAILQ_INSERT_TAIL(outbox, item, next);
    ESP_LOGD(TAG, "ENQUEUE msgid=%d, msg_type=%d, len=%d, size=%d", message->msg_id, message->msg_type, message->len + message->remaining_len, outbox_get_size(outbox));
    return item;
}

outbox_item_handle_t outbox_get(outbox_handle_t outbox, int msg_id)
{
    outbox_item_handle_t item;
    STAILQ_FOREACH(item, outbox, next) {
        if (item->msg_id == msg_id) {
            return item;
        }
    }
    return NULL;
}

outbox_item_handle_t outbox_dequeue(outbox_handle_t outbox, pending_state_t pending, outbox_tick_t *tick)
{
    outbox_item_handle_t item;
    STAILQ_FOREACH(item, outbox, next) {
        if (item->pending == pending) {
            if (tick) {
                *tick = item->tick;
            }
            return item;
        }
    }
    return NULL;
}

esp_err_t outbox_delete_item(outbox_handle_t outbox, outbox_item_handle_t item_to_delete)
{
    outbox_item_handle_t item;
    STAILQ_FOREACH(item, outbox, next) {
        if (item == item_to_delete) {
            STAILQ_REMOVE(outbox, item, outbox_item, next);
#ifdef USE_PSRAM
            heap_caps_free(item->buffer);
            heap_caps_free(item);
#else
            free(item->buffer);
            free(item);
#endif
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}

uint8_t *outbox_item_get_data(outbox_item_handle_t item,  size_t *len, uint16_t *msg_id, int *msg_type, int *qos)
{
    if (item) {
        *len = item->len;
        *msg_id = item->msg_id;
        *msg_type = item->msg_type;
        *qos = item->msg_qos;
        return (uint8_t *)item->buffer;
    }
    return NULL;
}

esp_err_t outbox_delete(outbox_handle_t outbox, int msg_id, int msg_type)
{
    outbox_item_handle_t item, tmp;
    STAILQ_FOREACH_SAFE(item, outbox, next, tmp) {
        if (item->msg_id == msg_id && (0xFF & (item->msg_type)) == msg_type) {
            STAILQ_REMOVE(outbox, item, outbox_item, next);
#ifdef USE_PSRAM
            heap_caps_free(item->buffer);
            heap_caps_free(item);
#else
            free(item->buffer);
            free(item);
#endif
            ESP_LOGD(TAG, "DELETED msgid=%d, msg_type=%d, remain size=%d", msg_id, msg_type, outbox_get_size(outbox));
            return ESP_OK;
        }

    }
    return ESP_FAIL;
}
esp_err_t outbox_delete_msgid(outbox_handle_t outbox, int msg_id)
{
    outbox_item_handle_t item, tmp;
    STAILQ_FOREACH_SAFE(item, outbox, next, tmp) {
        if (item->msg_id == msg_id) {
            STAILQ_REMOVE(outbox, item, outbox_item, next);
#ifdef USE_PSRAM
            heap_caps_free(item->buffer);
            heap_caps_free(item);
#else
            free(item->buffer);
            free(item);
#endif
        }

    }
    return ESP_OK;
}
esp_err_t outbox_set_pending(outbox_handle_t outbox, int msg_id, pending_state_t pending)
{
    outbox_item_handle_t item = outbox_get(outbox, msg_id);
    if (item) {
        item->pending = pending;
        return ESP_OK;
    }
    return ESP_FAIL;
}

pending_state_t outbox_item_get_pending(outbox_item_handle_t item)
{
    if (item) {
        return item->pending;
    }
    return QUEUED;
}

esp_err_t outbox_set_tick(outbox_handle_t outbox, int msg_id, outbox_tick_t tick)
{
    outbox_item_handle_t item = outbox_get(outbox, msg_id);
    if (item) {
        item->tick = tick;
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t outbox_delete_msgtype(outbox_handle_t outbox, int msg_type)
{
    outbox_item_handle_t item, tmp;
    STAILQ_FOREACH_SAFE(item, outbox, next, tmp) {
        if (item->msg_type == msg_type) {
            STAILQ_REMOVE(outbox, item, outbox_item, next);
#ifdef USE_PSRAM
            heap_caps_free(item->buffer);
            heap_caps_free(item);
#else
            free(item->buffer);
            free(item);
#endif
        }

    }
    return ESP_OK;
}
int outbox_delete_single_expired(outbox_handle_t outbox, outbox_tick_t current_tick, outbox_tick_t timeout)
{
    int msg_id = -1;
    outbox_item_handle_t item;
    STAILQ_FOREACH(item, outbox, next) {
        if (current_tick - item->tick > timeout) {
            STAILQ_REMOVE(outbox, item, outbox_item, next);

#ifdef USE_PSRAM
            heap_caps_free(item->buffer);
#else
            free(item->buffer);
#endif

            msg_id = item->msg_id;

#ifdef USE_PSRAM
            heap_caps_free(item);
#else
            free(item);
#endif

            return msg_id;
        }

    }
    return msg_id;
}

int outbox_delete_expired(outbox_handle_t outbox, outbox_tick_t current_tick, outbox_tick_t timeout)
{
    int deleted_items = 0;
    outbox_item_handle_t item, tmp;
    STAILQ_FOREACH_SAFE(item, outbox, next, tmp) {
        if (current_tick - item->tick > timeout) {
            STAILQ_REMOVE(outbox, item, outbox_item, next);
#ifdef USE_PSRAM
            heap_caps_free(item->buffer);
            heap_caps_free(item);
#else
            free(item->buffer);
            free(item);
#endif
            deleted_items ++;
        }

    }
    return deleted_items;
}

int outbox_get_size(outbox_handle_t outbox)
{
    int siz = 0;
    outbox_item_handle_t item;
    STAILQ_FOREACH(item, outbox, next) {
        // Suppressing "use after free" warning as this could happen only if queue is in inconsistent state
        // which never happens if STAILQ interface used
        siz += item->len; // NOLINT(clang-analyzer-unix.Malloc)
    }
    return siz;
}

void outbox_delete_all_items(outbox_handle_t outbox)
{
    outbox_item_handle_t item, tmp;
    STAILQ_FOREACH_SAFE(item, outbox, next, tmp) {
        STAILQ_REMOVE(outbox, item, outbox_item, next);
#ifdef USE_PSRAM
            heap_caps_free(item->buffer);
            heap_caps_free(item);
#else
            free(item->buffer);
            free(item);
#endif
    }
}
void outbox_destroy(outbox_handle_t outbox)
{
    outbox_delete_all_items(outbox);

#ifdef USE_PSRAM
    heap_caps_free(outbox);
#else
    free(outbox);
#endif
}

#endif /* CONFIG_MQTT_CUSTOM_OUTBOX */
