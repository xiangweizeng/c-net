/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include <stdlib.h>
#include "allocator.h"
#include "status.h"
#include "memory_cached.h"


#ifdef  MEMORY_POOL
static DATA_DRAM memory_cached_t memory_cached;
void alloc_memory_pool_init(size_t pool_size)
{
    memory_cached_create(&memory_cached, pool_size);
}

void alloc_memory_pool_deinit()
{
    memory_cached_release(&memory_cached);
}

void alloc_memory_pool_stat()
{
    memory_cached_stat(&memory_cached);
}

FUNCTION_IRAM void *fast_malloc(size_t size) {
    void *ptr = memory_cached_malloc(&memory_cached, size);
    return ptr;
}

FUNCTION_IRAM void fast_free(void *ptr) {
    if (ptr) {
       memory_cached_free(&memory_cached, ptr);
    }
}

FUNCTION_IRAM void *fast_malloc_align32(size_t size) {
    void *ptr = memory_cached_malloc(&memory_cached, size);
    return ptr;
}

FUNCTION_IRAM void fast_free_align32(void *ptr) {
    if (ptr) {
        memory_cached_free(&memory_cached, ptr);
    }
}

#else

void alloc_memory_pool_init(size_t pool_size){}
void alloc_memory_pool_deinit(){}
void alloc_memory_pool_stat(){}

#ifdef ESP32
#include "esp_heap_caps.h"

FUNCTION_IRAM void * fast_malloc(size_t size){
    return malloc(size);
}

FUNCTION_IRAM void fast_free(void* ptr){
    if(ptr) {
        heap_caps_free(ptr);
    }
}

/**
 * call default malloc, alloc size + 4
 * pre_size = 4 - ptr % 4
 * pre + pre_size
 * pre[-1] = pre_size
 * @param size
 * @return
 */
FUNCTION_IRAM  void *fast_malloc_align32(size_t size) {
    char *r = (char*)malloc(size + 4u);
    if(NULL != r){
        size_t pre = 4 - (size_t)r % 4;
        r += pre;
        r[-1] = pre;
    }
    return r;
}

/**
 * ptr[-1] pre_size
 * ptr - pre_size
 * @param ptr
 */
FUNCTION_IRAM void fast_free_align32(void *ptr) {
    if (ptr && 0 == (size_t)ptr % 4) {
        char *r = (char*)ptr;
        r -= r[-1];
        heap_caps_free(r);
    }
}

FUNCTION_IRAM void * extermal_memory_malloc(size_t size){
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

FUNCTION_IRAM esp_external_ram_allocator_t *esp_external_ram_allocator_create()
{
    DATA_DRAM static esp_external_ram_allocator_t  allocator;
    allocator.base.free = fast_free;
    allocator.base.free_align32 = fast_free;
    allocator.base.malloc = extermal_memory_malloc;
    allocator.base.malloc_align32 = extermal_memory_malloc;
    return  &allocator;
}

#else

FUNCTION_IRAM void *fast_malloc(size_t size) {
    void *ptr = NULL;
    size = align_size(size, 16);
    fast_memory_memalign(&ptr, 16, size);
    return ptr;
}

FUNCTION_IRAM void fast_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

FUNCTION_IRAM void *fast_malloc_align32(size_t size) {
    void *ptr = NULL;
    size = align_size(size, 16);
    fast_memory_memalign(&ptr, 16, size);
    return ptr;
}

FUNCTION_IRAM void fast_free_align32(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}
#endif

#endif

FUNCTION_IRAM int fast_memory_memalign(void **ptr, size_t align_size, size_t size) {

#if defined(__unix__)
    return posix_memalign(ptr, align_size, size);
#endif
    *ptr = malloc(size);

    return CNET_STATUS_SUCCESS;
}

FUNCTION_IRAM allocator_t *allocator_create() {
    DATA_DRAM static allocator_t allocator;
    allocator.malloc = fast_malloc;
    allocator.free = fast_free;
    allocator.malloc_align32 = fast_malloc_align32;
    allocator.free_align32 = fast_free_align32;
    allocator.memory_type = ALLOCATOR_MEMORY_COMMON_TYPE;
    return &allocator;
}

FUNCTION_IRAM malloc_func allocator_get_malloc_fun(allocator_t *allocator, int align32) {
    malloc_func malloc_function = NULL;
    if (NULL != allocator) {
        malloc_function = align32 == 1 ? allocator->malloc_align32 : allocator->malloc;
    }

    if (NULL == malloc_function) {
        malloc_function = align32 == 1 ? fast_malloc_align32 : fast_malloc;
    }

    return malloc_function;
}

FUNCTION_IRAM free_func allocator_get_free_fun(allocator_t *allocator, int align32) {
    free_func free_function = NULL;
    if (NULL != allocator) {
        free_function = align32 == 1 ? allocator->free_align32 : allocator->free;
    }

    if (NULL == free_function) {
        free_function = align32 == 1 ? fast_free_align32 : fast_free;
    }

    return free_function;
}

FUNCTION_IRAM allocator_memory_type_t allocator_get_memory_type(allocator_t *allocator)
{
    return  allocator != NULL ? allocator->memory_type : ALLOCATOR_MEMORY_COMMON_TYPE;
}
