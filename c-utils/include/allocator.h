/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_ALLOCATOR_H
#define CNET_ALLOCATOR_H

#include <string.h>
#include <malloc.h>

#define  STATIC_ALIGNED_64       __attribute__((aligned(64)))
#define  STATIC_ALIGNED_128      __attribute__((aligned(128)))
#define  STATIC_ALIGNED_256      __attribute__((aligned(256)))


#ifdef ESP32

#include "esp_attr.h"
#define  FUNCTION_IRAM      IRAM_ATTR           // function in iram
#define  DATA_DRAM          DRAM_ATTR           // data in dram
#define  DATA_RTC_UNINIT    RTC_NOINIT_ATTR     // data in rtc memory
#define  DATA_DRAM_UNINIT   __NOINIT_ATTR       // data unset after reboot
#define  DATA_ERAM          EXT_RAM_ATTR        // data in ext ram
#define  ATOMIC(name)       _Atomic(name)
#else
#define  FUNCTION_IRAM
#define  DATA_DRAM
#define  DATA_RTC_UNINIT
#define  DATA_DRAM_UNINIT
#define  DATA_ERAM
#define  ATOMIC(name)  name _Atomic
#endif


/**
 * exchange-add operation for atomic operations on reference counters
 */
#define CNET_XADD(addr, delta) (int)__sync_fetch_and_add((unsigned*)(addr), (unsigned)(delta))

/**
 * Aligns a buffer size to the specified number of bytes
 * The function returns the minimum number that is greater or equal to sz and is divisible by n
 * sz Buffer size to align
 * n Alignment size that must be a power of two
 * @param sz
 * @param n
 * @return
 */
FUNCTION_IRAM static inline size_t align_size(size_t sz, int n) {
    return (sz + n - 1) & -n;
}

/**
 * init memory pool
 * @param pool_size
 */
void alloc_memory_pool_init(size_t pool_size);

/**
 * deinit memory pool
 */
void alloc_memory_pool_deinit();

/**
 * print memory pool stat
 */
void alloc_memory_pool_stat();

/**
 * fast malloc memory
 * @param size
 * @return
 */
FUNCTION_IRAM void * fast_malloc(size_t size);

/**
 * fast free memory
 * @param ptr
 */
FUNCTION_IRAM void fast_free(void* ptr);

/**
 * fast malloc align32 memory
 * @param size
 * @return
 */
FUNCTION_IRAM  void *fast_malloc_align32(size_t size);

/**
 * fast free align32 memory
 * @param ptr
 */
FUNCTION_IRAM void fast_free_align32(void *ptr);

/**
 * alloc memory for align
 * @param ptr
 * @param align_size
 * @param size
 */
FUNCTION_IRAM int fast_memory_memalign(void **ptr, size_t  align_size,  size_t size);


/**
 * allocator type
 */
typedef enum allocator_memory_type_t{
    /**
     * general memory
     */
    ALLOCATOR_MEMORY_COMMON_TYPE = 0,

    /**
     * k210 kpu chip memory
     */
    ALLOCATOR_K210KPU_CHIP_MEMORY_TYPE = 1,

    /**
     * esp32 external ram
     */
    ALLOCATOR_ESP32_EXTERNAL_RAM_MEMORY_TYPE = 2,
}allocator_memory_type_t;

/**
 * memory operation interface, malloc for allocator
 */
typedef void *(*malloc_func)(size_t size);

/**
 * memory operation intrface, free for allocator
 */
typedef void (*free_func)(void*);

/**
 * base allocator
 */
typedef struct allocator_t{

    /**
     * allocator memory type
     */
    allocator_memory_type_t memory_type;

    /**
     * malloc interface
     */
    malloc_func malloc;

    /**
     * free interface
     */
    free_func free;

    /**
     * malloc align32 memory interface
     */
    malloc_func malloc_align32;

    /**
     * free align32 memory interface
     */
    free_func free_align32;
}allocator_t;

/**
 * default allocator, use fast_malloc and fast_free
 * singleton pattern, not release
 * @return default allocator
 */
FUNCTION_IRAM allocator_t *allocator_create();

/**
 * get malloc function from allocator
 * @param allocator
 * @param mode
 * @return , if null, return default
 */
FUNCTION_IRAM malloc_func allocator_get_malloc_fun(allocator_t *allocator, int align32);

/**
 * get malloc function from allocator
 * @param allocator
 * @param mode
 * @return if null return default
 */
FUNCTION_IRAM free_func allocator_get_free_fun(allocator_t *allocator, int align32);

/**
 * get allocator memory type
 * @param allocator
 * @return
 */
FUNCTION_IRAM allocator_memory_type_t allocator_get_memory_type(allocator_t *allocator);

#ifdef ESP32

/**
 * esp external ram allocator
 *
 */
typedef struct esp_external_ram_allocator_t{
    allocator_t base;
}esp_external_ram_allocator_t;

/**
 * create external ram allocator
 * singleton pattern, not release
 * @return
 */
FUNCTION_IRAM esp_external_ram_allocator_t *esp_external_ram_allocator_create();

#endif

#endif //CNET_ALLOCATOR_H
