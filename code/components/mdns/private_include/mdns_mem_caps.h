/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocate memory.
 * @param size Number of bytes to allocate.
 * @return Pointer to allocated memory, or NULL on failure.
 */
void *mdns_mem_malloc(size_t size);

/**
 * @brief Allocate and zero memory.
 * @param num Number of elements.
 * @param size Size of each element.
 * @return Pointer to allocated memory, or NULL on failure.
 */
void *mdns_mem_calloc(size_t num, size_t size);

/**
 * @brief Free allocated memory.
 * @param ptr Pointer to memory to free.
 */
void mdns_mem_free(void *ptr);

/**
 * @brief Duplicate a string.
 * @param s String to duplicate.
 * @return Pointer to duplicated string, or NULL on failure.
 */
char *mdns_mem_strdup(const char *s);

/**
 * @brief Duplicate a string with length limit.
 * @param s String to duplicate.
 * @param n Maximum number of characters to copy.
 * @return Pointer to duplicated string, or NULL on failure.
 */
char *mdns_mem_strndup(const char *s, size_t n);

/**
 * @brief Allocate memory for mDNS task.
 * @param size Number of bytes to allocate.
 * @return Pointer to allocated memory, or NULL on failure.
 */
void *mdns_mem_task_malloc(size_t size);

/**
 * @brief Free allocated memory for mDNS task.
 * @param ptr Pointer to memory to free.
 */
void mdns_mem_task_free(void *ptr);

#ifdef __cplusplus
}
#endif
