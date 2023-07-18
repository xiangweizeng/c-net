/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "vector_dot_int8_benchmark.h"
#include "vector_dot_int8.h"
#include "allocator.h"

BENCHMARKS(compute_vector_dot){
        {.name = "compute_vector_dot-10000",     .args = {10000 ,  .a = NULL, .b = NULL}},
        {.name = "compute_vector_dot-20000",     .args = {20000 , .a = NULL, .b = NULL}},
        {.name = "compute_vector_dot-40000",     .args = {40000 , .a = NULL, .b = NULL}},
        {.name = "compute_vector_dot-80000",     .args = {80000 , .a = NULL, .b = NULL}},
        {.name = "compute_vector_dot-160000",    .args = {160000 , .a = NULL, .b = NULL}},
        {.name = "compute_vector_dot-320000",    .args = {320000 , .a = NULL, .b = NULL}},
};

static benchmark_status_t compute_vector_dot_benchmark(vector_add_create_args_t *args){
    compute_vector_dot(args->a, args->b, args->vector_len);
    return BENCH_MARK_SUCCESS;
}

static benchmark_status_t compute_vector_dot_setup(vector_add_create_args_t *args){
    args->a = fast_malloc(args->vector_len);
    args->b = fast_malloc(args->vector_len);
    return BENCH_MARK_SUCCESS;
}

static benchmark_status_t compute_vector_dot_teardown(vector_add_create_args_t *args){
    if(args->a != NULL){
        fast_free(args->a);
        args->a = NULL;
    }

    if(args->b != NULL){
        fast_free(args->b);
        args->b = NULL;
    }

    return BENCH_MARK_SUCCESS;
}

BENCHMARK_IMPL(compute_vector_dot, vector_add_create_args_t, compute_vector_dot_benchmark)

void benchmark_vector_dot()
{
    BENCHMARK_SET_SETUP(compute_vector_dot, compute_vector_dot_setup);
    BENCHMARK_SET_TEAR_DOWN(compute_vector_dot, compute_vector_dot_teardown);
    BENCHMARKS_RUN(compute_vector_dot);
}
