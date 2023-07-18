/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_BENCHMARK_FUNCTION_H
#define CNET_BENCHMARK_FUNCTION_H

#include "benchmark.h"
#include <string.h>

/**
 * benchmark_old declare for function benchmark_old
 */
#define BENCHMARK_DECLARE(impl, create_args_t)\
void run_##impl##_benchmark();\
typedef benchmark_status_t (*impl##_benchmark_function)(create_args_t *);\
void benchmark_##impl_set_setup(impl##_benchmark_function);\
void benchmark_##impl_set_tear_down(impl##_benchmark_function);\
typedef struct benchmark_##impl##_t{\
    char name[256];\
    create_args_t args;\
}benchmark_##impl##_t;

/**
 * run function benchmarks
 */
#define BENCHMARKS_RUN(impl) run_##impl##_benchmarks()

/**
 * set function benchmark_old setup
 */
#define BENCHMARK_SET_SETUP(impl, fun) benchmark_##impl##_set_setup(fun)

/**
 * set function benchmark_old tear down
 */
#define BENCHMARK_SET_TEAR_DOWN(impl, fun) benchmark_##impl##_set_tear_down(fun)

/**
 * impl benchmarks, name and args
 */
#define BENCHMARKS(impl) \
benchmark_##impl##_t impl##_benchmarks[] =

/**
 * function benchmark_old impl
 */
#define BENCHMARK_IMPL(impl, create_args_t, test_function)                  \
static benchmark_status_t impl##_create_context(benchmark_t* benchmark)     \
{                                                                           \
    benchmark->context = test_function;                                     \
    return BENCH_MARK_SUCCESS;                                              \
}                                                                           \
                                                                            \
static impl##_benchmark_function impl##_benchmark_setup_fun = NULL;         \
static impl##_benchmark_function impl##_benchmark_tear_down_fun = NULL;     \
static benchmark_status_t impl##_benchmark_pre_run(benchmark_t* benchmark)  \
{                                                                           \
    create_args_t *args = (create_args_t*)benchmark->create_args;           \
    if(NULL != impl##_benchmark_setup_fun){                                 \
        return impl##_benchmark_setup_fun(args);                            \
        }                                                                   \
    return  BENCH_MARK_SUCCESS;                                             \
}                                                                           \
                                                                            \
static benchmark_status_t impl##_benchmark_run(benchmark_t* benchmark)      \
{                                                                           \
    if(NULL == benchmark->context){                                         \
        return BENCH_MARK_INVALID;                                          \
    }                                                                       \
                                                                            \
    create_args_t *args = (create_args_t*)benchmark->create_args;           \
    ((impl##_benchmark_function)(benchmark->context))(args);                \
    return  BENCH_MARK_SUCCESS;                                             \
}                                                                           \
                                                                            \
static benchmark_status_t impl##_benchmark_post_run(benchmark_t* benchmark) \
{                                                                           \
    create_args_t *args = (create_args_t*)benchmark->create_args;           \
    if(NULL != impl##_benchmark_tear_down_fun){                             \
        return impl##_benchmark_tear_down_fun(args);                        \
    }                                                                       \
    return  BENCH_MARK_SUCCESS;                                             \
}                                                                           \
                                                                            \
benchmark_t benchmark_create_##impl(benchmark_##impl##_t *example)          \
{                                                                           \
    benchmark_t benchmark= {                                                \
            .run_status = BENCH_MARK_INVALID,                               \
            .iteration = 10,                                                \
            .create_args = &example->args,                                  \
            .context = NULL,                                                \
            .create_context =  impl##_create_context,                       \
            .pre_run = impl##_benchmark_pre_run,                            \
            .run = impl##_benchmark_run,                                    \
            .post_run = impl##_benchmark_post_run,                          \
            .release_context = NULL                                         \
    };                                                                      \
                                                                            \
    strcpy(benchmark.benchmark_name, example->name);                        \
    return benchmark;                                                       \
}                                                                           \
                                                                            \
void benchmark_##impl##_set_setup(impl##_benchmark_function fun)            \
{                                                                           \
    impl##_benchmark_setup_fun = fun;                                       \
}                                                                           \
                                                                                    \
void benchmark_##impl##_set_tear_down(impl##_benchmark_function fun)                \
{                                                                                   \
    impl##_benchmark_tear_down_fun = fun;                                           \
}                                                                                   \
                                                                                    \
void run_##impl##_benchmarks()                                                      \
{                                                                                   \
    int size = sizeof(impl##_benchmarks) / (sizeof(struct benchmark_##impl##_t));   \
    for(int i= 0 ; i < size; i ++){                                                 \
        benchmark_t benchmark = benchmark_create_##impl(&(impl##_benchmarks[i]));   \
        benchmark_statistical_run(&benchmark);                                      \
    }                                                                               \
}

#endif //CNET_BENCHMARK_FUNCTION_H
