/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "option.h"
#include "allocator.h"
#include "stdio.h"

#define CNET_MEMORY_DEFAULT_POOL_SIZE 4096000

DATA_DRAM static option_t option = {
        .light_mode = 0,
        .thread_number = 1,
        .memory_pool_size = CNET_MEMORY_DEFAULT_POOL_SIZE
};

FUNCTION_IRAM void option_init(int thread_number)
{
    alloc_memory_pool_init(option.memory_pool_size);
    alloc_memory_pool_stat();

    if(option.pool != NULL){
        pthreadpool_destroy(option.pool);
    }

    option.thread_number = thread_number;
    option.tensor = allocator_create();
    option.workspace = allocator_create();
    option.pool = pthreadpool_create(option.thread_number);
    if(option.pool == NULL){
        printf("create potheads failed\n");
    }

    option.inited = 1;
}


FUNCTION_IRAM void option_uninit()
{
    if(option.pool != NULL){
        pthreadpool_destroy(option.pool);
        option.pool = NULL;
    }

    alloc_memory_pool_deinit();
}


FUNCTION_IRAM option_t option_get_instance()
{
    if(!option.inited){
        option_init(option.thread_number);
    }

    return option;
}

