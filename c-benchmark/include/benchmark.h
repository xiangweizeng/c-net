/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_BENCHMARK_H
#define CNET_BENCHMARK_H

#include <stdlib.h>
#include <float.h>

#undef Min
#undef Max
#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))

/**
 * impl status
 */
typedef enum benchmark_status_t{
    /**
     * invalid
     */
    BENCH_MARK_INVALID = -1,

    /**
     * benmark run success
     */
    BENCH_MARK_SUCCESS = 0,

    /**
     * create context failed
     */
    BENCH_MARK_CREATE_CONTEXT_FAILED = 1,

    /**
     * prerun failed
     */
    BENCH_MARK_PRE_RUN_FAILED = 2,

    /**
     * run failed
     */
    BENCH_MARK_RUN_FAILED = 3,

    /**
     * postrun failed
     */
    BENCH_MARK_POST_RUN_FAILED = 4,

    /**
     * release context failed
     */
    BENCH_MARK_RELEASE_CONTEXT_FAILED = 5,
}benchmark_status_t;

/**
 * benchmakr_t
 *
 * basic data
 * contxt
 * function
 */
struct benchmark_t;
typedef struct benchmark_t benchmark_t;

/**
 * benchmakr function
 */
typedef benchmark_status_t (*benchmark_run)(benchmark_t* benchmark);


struct benchmark_t{
    /**
     * basic info
     */
    char benchmark_name[256];
    int iteration;
    void *create_args;

    /**
     * statistical info
     */
    double time_min;
    double time_max;
    double time_total;
    benchmark_status_t run_status;

    /**
     * context, create by create_cotext, release by release release_context
     */
    void *context;

    /**
     * create context function
     */
    benchmark_run create_context;

    /**
     * create name from create_args and bencmark
     */
    benchmark_run create_benchmark_name;

    /**
     * pre run function, call every run
     */
    benchmark_run pre_run;

    /**
     * run function
     */
    benchmark_run run;

    /**
     * post run function
     */
    benchmark_run post_run;

    /**
     * release contxt function
     */
    benchmark_run release_context;
};

/**
 * create default impl
 * @return
 */
static inline benchmark_t benchmark_create_default()
{
    benchmark_t benchmark = {
            .benchmark_name ={0},
            .iteration = 0,
            .create_args = NULL,
            .time_max = 0,
            .time_min = 0,
            .time_total = 0,
            .context = NULL,
            .create_context = NULL,
            .pre_run = NULL,
            .run = NULL,
            .post_run = NULL,
            .release_context = NULL
    };

    return benchmark;
}

/**
 * run impl get statistical data
 * @param benchmark
 * @return
 */
benchmark_status_t benchmark_statistical_run(benchmark_t *benchmark);

#endif //CNET_BENCHMARK_H
