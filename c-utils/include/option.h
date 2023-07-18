/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_OPTION_H
#define CNET_OPTION_H

#include "allocator.h"
#include "pthreadpool.h"


typedef struct option_t {
    /**
     * have inited
     */
    int inited;

    /**
     * light mode, intermediate blob will be recycled when enabled, enabled by default
     */
    int light_mode;

    /**
     * thread pool number
     */
    int thread_number;

    /**
     * common memory pool size
     */
    size_t memory_pool_size;

    /**
     * thread handle
     */
    pthreadpool_t pool;

    /**
     * tensor data allocator
     */
    allocator_t *tensor;

    /**
     * workspace data allocator
     */
    allocator_t *workspace;
} option_t;

/**
 * init  option
 * @return
 */
FUNCTION_IRAM void option_init(int thread_number);

/**
 * uninit  option
 * @return
 */
FUNCTION_IRAM void option_uninit();

/**
 * get odefault option
 * @return
 */
FUNCTION_IRAM option_t option_get_instance();

/**
 * parallelize for 1d
 */
#define PARALLELIZE_1D(func, context, size) \
    pthreadpool_parallelize_1d(opt->pool, (pthreadpool_task_1d_t)(func), &(context), size, 0)

/**
* parallelize for 1d tile 1d
*/
#define PARALLELIZE_1D_TILE_1D(func, context, size, tile) \
    pthreadpool_parallelize_1d_tile_1d(opt->pool, (pthreadpool_task_1d_tile_1d_t)(func), &(context), size, tile, 0u)

/**
 * parallelize for 2d
 */
#define PARALLELIZE_2D(func, context, x_size, y_size) \
    pthreadpool_parallelize_2d(opt->pthreadpool, (pthreadpool_task_2d_t)(func), &(context), x_size,  y_size, 0)

#endif //CNET_OPTION_H
