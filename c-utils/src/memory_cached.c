/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "memory_cached.h"

static DATA_DRAM ncx_slab_pool_t *mempool;

memory_cached_node_t *memory_cached_node_create(void *ptr, size_t size) {
    memory_cached_node_t *node = (memory_cached_node_t *) ncx_slab_alloc_locked(mempool, sizeof(memory_cached_node_t));
    node->linker = create_linked_node();
    node->address = ptr;
    node->size = size;
    return node;
}

void memory_cached_create(memory_cached_t *memory_cached, size_t pool_size) {
    u_char *space;
    space = (u_char *) malloc(pool_size);

    mempool = (ncx_slab_pool_t *) space;
    mempool->addr = space;
    mempool->min_shift = 3;
    mempool->end = space + pool_size;

    ncx_slab_init(mempool);
    memory_cached->mutex = ncx_shmtx_create();
    for (size_t i = 0; i < MEMORY_CACHED_SMALL_HASH_SIZE; i++) {
        memory_cached->small_free_blocks[i] = create_linked_list(NULL);
    }

    for (size_t i = 0; i < MEMORY_CACHED_BIG_HASH_SIZE; i++) {
        memory_cached->big_free_blocks[i] = create_linked_list(NULL);
    }

    for (size_t i = 0; i < MEMORY_CACHED_USING_HASH_SIZE; i++) {
        memory_cached->using_blocks[i] = create_linked_list(NULL);
    }
}

static void memory_cached_recycle_locked(memory_cached_t *memory_cached);
static inline memory_cached_node_t *malloc_cacched_memory(memory_cached_t *memory_cached, size_t size) {
    memory_cached_node_t *free_memory_node;
    void * ptr = ncx_slab_alloc_locked(mempool, size);
    if (ptr == NULL){
        printf("malloc_cacched_memory recycle\n");
        memory_cached_recycle_locked(memory_cached);
        ptr = ncx_slab_alloc_locked(mempool, size);
    }

    free_memory_node = memory_cached_node_create(ptr, size);
    return free_memory_node;
}

static inline void cached_memory_free(memory_cached_node_t *free_node) {
    if (free_node->address != NULL && free_node->size > 0) {
        ncx_slab_free_locked(mempool, free_node->address);
    }
    ncx_slab_free_locked(mempool, free_node);
}

static void free_linked_list_memory_blocks(linked_list *memory_list) {
    linked_node *current = memory_list->head;
    while (current != NULL) {
        memory_cached_node_t *memory_node = (memory_cached_node_t *) current;

        memory_cached_node_t *free_node = memory_node;
        current = current->next;

        cached_memory_free(free_node);
    }
}

void memory_cached_release(memory_cached_t *memory_cached) {
    memory_cached_recycle(memory_cached);

    for (size_t i = 0; i < MEMORY_CACHED_USING_HASH_SIZE; i++) {
        free_linked_list_memory_blocks(&memory_cached->using_blocks[i]);
    }

    ncx_shmtx_release(&mempool->mutex);
    free(mempool->addr);
    mempool = NULL;

    ncx_shmtx_release(&memory_cached->mutex);
}

static inline int find_memcached_node_by_size(memory_cached_node_t *node, void *args) {
    size_t size = *(size_t *) args;
    if (node != NULL && node->size == size) {
        return 1;
    }
    return 0;
}

static inline int find_memcached_node_by_address(memory_cached_node_t *node, void *args) {
    if (node != NULL && node->address == args) {
        return 1;
    }
    return 0;
}


static void memory_cached_recycle_locked(memory_cached_t *memory_cached) {
    for (size_t i = 0; i < MEMORY_CACHED_SMALL_HASH_SIZE; i++) {
        free_linked_list_memory_blocks(&memory_cached->small_free_blocks[i]);
    }

    for (size_t i = 0; i < MEMORY_CACHED_BIG_HASH_SIZE; i++) {
        free_linked_list_memory_blocks(&memory_cached->big_free_blocks[i]);
    }

    ncx_slab_stat_t stat;
    ncx_slab_stat(mempool, &stat);
}

static void *memory_cached_malloc_locked(memory_cached_t *memory_cached, size_t size) {
    memory_cached_node_t *free_memory_node = NULL;

    size = align_size(size, MEMORY_CACHED_SMALL_MIN_BLOCK_SIZE);
    if(size <= MEMORY_CACHED_SMALL_MAX_BLOCK_SIZE)
    {
        size_t hash = size / MEMORY_CACHED_SMALL_MIN_BLOCK_SIZE - 1;
        linked_list * current_hash = &(memory_cached->small_free_blocks[hash]);

        if(current_hash->linked_size > 0){
            free_memory_node = (memory_cached_node_t*)pop_linked_list_head(current_hash);
        }
    }else{
        size_t hash = size % MEMORY_CACHED_BIG_HASH_SIZE;
        linked_list * current_hash = &(memory_cached->big_free_blocks[hash]);

        size_t find_size = size;
        free_memory_node = (memory_cached_node_t *)linked_list_find_node(current_hash,
                                                                         (find_node)find_memcached_node_by_size, &find_size);
        if(NULL != free_memory_node){
            linked_list_delete_node(current_hash, (linked_node*)free_memory_node);
        }
    }

    if(NULL == free_memory_node){
        free_memory_node = malloc_cacched_memory(memory_cached, size);
    }else{
        memory_cached->total_free_size += free_memory_node->size;
    }

    if(NULL != free_memory_node && free_memory_node->address != NULL && free_memory_node->size > 0){
        size_t using_hash = ((size_t)free_memory_node->address) % MEMORY_CACHED_USING_HASH_SIZE;
        linked_list *current_hash = &(memory_cached->using_blocks[using_hash]);
        linked_list_insert_tail(current_hash, (linked_node*)free_memory_node);
    }

    if(free_memory_node == NULL || free_memory_node->address == NULL){
        printf("memory_cached_malloc_locked failed\n");
        if(free_memory_node != NULL){
            ncx_slab_free_locked(mempool, free_memory_node);
        }
        return NULL;
    }

    return free_memory_node->size > 0 ? free_memory_node->address : NULL;
}

void *memory_cached_malloc(memory_cached_t *memory_cached, size_t size) {
    void *p;
    ncx_shmtx_lock(memory_cached->mutex);
    p = memory_cached_malloc_locked(memory_cached, size);
    ncx_shmtx_unlock(memory_cached->mutex);

    return p;
}

static void memory_cached_free_locked(memory_cached_t *memory_cached, void *ptr) {
    size_t using_hash = ((size_t) ptr) % MEMORY_CACHED_USING_HASH_SIZE;
    linked_list *current_hash = &(memory_cached->using_blocks[using_hash]);

    void *find_address = ptr;
    memory_cached_node_t *using_node = NULL;

    using_node = (memory_cached_node_t *) linked_list_find_node(current_hash,
                                                                (find_node) find_memcached_node_by_address,
                                                                find_address);
    if (using_node != NULL) {
        linked_list_delete_node(current_hash, (linked_node *) using_node);

        if(using_node->size > MEMORY_CACHED_BLOCK_SIZE_THRESHOLD ||
           memory_cached->total_free_size > MEMORY_CACHED_RECYCLE_THRESHOLD)
        {
            cached_memory_free((memory_cached_node_t*)using_node);
        } else
            {
            size_t threshold = MEMORY_CACHED_BLOCK_SIZE_THRESHOLD;
            if (using_node->size <= MEMORY_CACHED_SMALL_MAX_BLOCK_SIZE) {
                size_t free_hash = using_node->size / MEMORY_CACHED_SMALL_MIN_BLOCK_SIZE - 1;
                threshold = MEMORY_CACHED_SMALL_TOTAL_THRESHOLD;
                current_hash = &(memory_cached->small_free_blocks[free_hash]);
            } else {
                size_t free_hash = using_node->size % MEMORY_CACHED_BIG_HASH_SIZE;
                current_hash = &(memory_cached->big_free_blocks[free_hash]);
            }

            size_t current_size = using_node->size*current_hash->linked_size;
            if(current_size > threshold){
                cached_memory_free((memory_cached_node_t*)using_node);
            }else{
                memory_cached->total_free_size += using_node->size;
                linked_list_insert_tail(current_hash, (linked_node *) using_node);
            }
        }

    } else {
        if ((u_char *) ptr < mempool->start || (u_char *) ptr > mempool->end) {
            free(ptr);
        } else {
            printf("alarm: invaild memory free\n");
            ncx_slab_free_locked(mempool, ptr);
        }
    }
}

void memory_cached_free(memory_cached_t *memory_cached, void *ptr) {
    ncx_shmtx_lock(memory_cached->mutex);
    memory_cached_free_locked(memory_cached, ptr);
    ncx_shmtx_unlock(memory_cached->mutex);
}

void memory_cached_recycle(memory_cached_t *memory_cached) {
    ncx_shmtx_lock(memory_cached->mutex);
    memory_cached_recycle_locked(memory_cached);
    ncx_shmtx_unlock(memory_cached->mutex);
}

void memory_cached_stat(memory_cached_t *memory_cached) {
    ncx_slab_stat_t stat;
    ncx_slab_stat(mempool, &stat);
}