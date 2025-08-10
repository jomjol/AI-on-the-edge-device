/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include "sdkconfig.h"
#include "mdns_private.h"
#include "mdns_mem_caps.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

#if CONFIG_MDNS_MEMORY_CUSTOM_IMPL
#define ALLOW_WEAK __attribute__((weak))
#else
#define ALLOW_WEAK
#endif

#if CONFIG_MDNS_TASK_CREATE_FROM_SPIRAM
#define MDNS_TASK_MEMORY_CAPS (MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#define MDNS_TASK_MEMORY_LOG "SPIRAM"
#endif
#if CONFIG_MDNS_TASK_CREATE_FROM_INTERNAL
#define MDNS_TASK_MEMORY_CAPS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#define MDNS_TASK_MEMORY_LOG "internal RAM"
#endif

#if CONFIG_MDNS_MEMORY_ALLOC_SPIRAM
#define MDNS_MEMORY_CAPS (MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#endif
#if CONFIG_MDNS_MEMORY_ALLOC_INTERNAL
#define MDNS_MEMORY_CAPS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#endif

// Allocate memory from internal heap as default.
#ifndef MDNS_MEMORY_CAPS
#warning "No memory allocation method defined, using internal memory"
#define MDNS_MEMORY_CAPS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#endif
#ifndef MDNS_TASK_MEMORY_CAPS
#define MDNS_TASK_MEMORY_CAPS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#define MDNS_TASK_MEMORY_LOG "internal RAM"
#endif

void ALLOW_WEAK *mdns_mem_malloc(size_t size)
{
    return heap_caps_malloc(size, MDNS_MEMORY_CAPS);
}

void ALLOW_WEAK *mdns_mem_calloc(size_t num, size_t size)
{
    return heap_caps_calloc(num, size, MDNS_MEMORY_CAPS);
}

void ALLOW_WEAK mdns_mem_free(void *ptr)
{
    heap_caps_free(ptr);
}

char ALLOW_WEAK *mdns_mem_strdup(const char *s)
{
    if (!s) {
        return NULL;
    }
    size_t len = strlen(s) + 1;
    char *copy = (char *)heap_caps_malloc(len, MDNS_MEMORY_CAPS);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}

char ALLOW_WEAK *mdns_mem_strndup(const char *s, size_t n)
{
    if (!s) {
        return NULL;
    }
    size_t len = strnlen(s, n);
    char *copy = (char *)heap_caps_malloc(len + 1, MDNS_MEMORY_CAPS);
    if (copy) {
        memcpy(copy, s, len);
        copy[len] = '\0';
    }
    return copy;
}

void ALLOW_WEAK *mdns_mem_task_malloc(size_t size)
{
    ESP_LOGI("mdns_mem", "mDNS task will be created from %s", MDNS_TASK_MEMORY_LOG);
    return heap_caps_malloc(size, MDNS_TASK_MEMORY_CAPS);
}

void ALLOW_WEAK mdns_mem_task_free(void *ptr)
{
    heap_caps_free(ptr);
}
