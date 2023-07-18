/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_MEMORY_CACHED_H
#define CNET_MEMORY_CACHED_H

#include "container_linked_list.h"
#include "ncx-pool/ncx_slab.h"

typedef struct memory_cached_node_t{
    linked_node linker;
    void* address;
    size_t size;
}memory_cached_node_t;

/**
 * create memory_cached_node_t
 * @param ptr
 * @param size
 * @return
 */
memory_cached_node_t *memory_cached_node_create(void *ptr, size_t size);

#define MEMORY_CACHED_SMALL_HASH_SIZE           37
#define MEMORY_CACHED_SMALL_MIN_BLOCK_SIZE      8
#define MEMORY_CACHED_SMALL_MAX_BLOCK_SIZE      (MEMORY_CACHED_SMALL_HASH_SIZE*MEMORY_CACHED_SMALL_MIN_BLOCK_SIZE)
#define MEMORY_CACHED_BIG_HASH_SIZE             17
#define MEMORY_CACHED_USING_HASH_SIZE           251
#define MEMORY_CACHED_BLOCK_SIZE_THRESHOLD      409600
#define MEMORY_CACHED_SMALL_TOTAL_THRESHOLD     4096
#define MEMORY_CACHED_RECYCLE_THRESHOLD         (1024000*2)

typedef struct memory_cached_t{
    /**
     * lock for mutil thread
     */
    ncx_shmtx_t mutex;

    /**
     * the total free blocks size
     */
    size_t total_free_size;

    /**
     * directly hash size MEMORY_CACHED_SMALL_HASH_SIZE, hash = size
     */
    linked_list small_free_blocks[MEMORY_CACHED_SMALL_HASH_SIZE];

    /**
     * hash table size MEMORY_CACHED_BIG_HASH_SIZE, hashkey = size % MEMORY_CACHED_BIG_HASH_SIZE
     */
    linked_list big_free_blocks[MEMORY_CACHED_BIG_HASH_SIZE];

    /**
     * hash table MEMORY_CACHED_USING_HASH_SIZE, hashey = address % MEMORY_CACHED_USING_HASH_SIZE
     */
    linked_list using_blocks[MEMORY_CACHED_USING_HASH_SIZE];
}memory_cached_t;

/**
 * create memory_cached_t
 * @param memory_cached
 * @param pool_size
 */
void memory_cached_create(memory_cached_t* memory_cached, size_t pool_size);

/**
 * release memory_cached_t
 * @return
 */
void memory_cached_release(memory_cached_t* memory_cached);

/**
 * malloc memory from memory cached
 * @param memory_cached
 * @param size
 * @return
 */
void* memory_cached_malloc(memory_cached_t *memory_cached, size_t size);

/**
 * free memory to memory cached
 * @param memory_cached
 * @param size
 * @return
 */
void memory_cached_free(memory_cached_t *memory_cached, void*ptr);

/**
 * recycle memory from memory cached
 * @param memory_cached
 * @param size
 * @return
 */
void memory_cached_recycle(memory_cached_t *memory_cached);

/**
 * memory cached stat
 * @param memory_cached
 */
void memory_cached_stat(memory_cached_t *memory_cached);

#endif //CNET_MEMORY_CACHED_H
