/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <stdio.h>
#include "benchmark.h"
#include "timer.h"

void benchmark_release_context(benchmark_t *benchmark) {
    if(NULL != benchmark->release_context){
        benchmark->release_context(benchmark);
    }
}

/**
 * run impl get statistical data
 * @param benchmark
 * @return
 */
benchmark_status_t benchmark_statistical_run(benchmark_t *benchmark)
{
    if(NULL == benchmark){
        return BENCH_MARK_INVALID;
    }

    /**
     * create context
     */
    if(NULL != benchmark->create_context && BENCH_MARK_SUCCESS != benchmark->create_context(benchmark)){
        benchmark->run_status = BENCH_MARK_CREATE_CONTEXT_FAILED;
        return BENCH_MARK_CREATE_CONTEXT_FAILED;
    }

    double time_min = DBL_MAX;
    double time_max = -DBL_MAX;
    double time_total = 0;

    /**
     * run iteriation
     */
    for (int i = 0; i < benchmark->iteration; i++) {

        /**
         * pre run
         */
        if(NULL != benchmark->pre_run && BENCH_MARK_SUCCESS != benchmark->pre_run(benchmark)){
            benchmark->run_status = BENCH_MARK_PRE_RUN_FAILED;
            benchmark_release_context(benchmark);
            return BENCH_MARK_PRE_RUN_FAILED;
        }

        /**
         * run
         */
        double start = what_time_is_it_now();
        if(BENCH_MARK_SUCCESS != benchmark->run(benchmark)){
            benchmark->run_status = BENCH_MARK_RUN_FAILED;
            benchmark_release_context(benchmark);
            return BENCH_MARK_RUN_FAILED;
        }

        double end = what_time_is_it_now();
        double time = end - start;

        /**
         * post run
         */
        if(NULL != benchmark->post_run && BENCH_MARK_SUCCESS != benchmark->post_run(benchmark)){
            benchmark_release_context(benchmark);
            benchmark->run_status = BENCH_MARK_POST_RUN_FAILED;
            return BENCH_MARK_POST_RUN_FAILED;
        }

        /**
         * get run time, min, max, total
         */
        time_min = Min(time_min, time);
        time_max = Max(time_max, time);
        time_total += time;
    }

    /**
     * release context
     */
    if(NULL != benchmark->release_context && BENCH_MARK_SUCCESS != benchmark->release_context(benchmark)){
        benchmark->run_status = BENCH_MARK_RELEASE_CONTEXT_FAILED;
        return BENCH_MARK_RELEASE_CONTEXT_FAILED;
    }

    benchmark->time_min = time_min;
    benchmark->time_max = time_max;
    benchmark->time_total = time_total;
    benchmark->run_status = BENCH_MARK_SUCCESS;

    fprintf(stderr, "%s min = %7.2f  max = %7.2f  avg = %7.2f\n",
            benchmark->benchmark_name, benchmark->time_min, benchmark->time_max, benchmark->time_total / benchmark->iteration);

    return BENCH_MARK_SUCCESS;
}