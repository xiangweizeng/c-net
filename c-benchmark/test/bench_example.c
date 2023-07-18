/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "benchmark_function.h"

int add(int a, int b)
{
    return a+b;
}

typedef struct add_create_args_t{
    int a;
    int b;
}add_create_args_t;

benchmark_status_t test_add(add_create_args_t *args){
    add(args->a, args->b);
    return BENCH_MARK_SUCCESS;
}

BENCHMARK_DECLARE(add, add_create_args_t)

BENCHMARKS(add){
        {.name = "add", .args = { 1 ,  1}},
        {.name = "add", .args = {-1 , -2}},
};

BENCHMARK_IMPL(add, add_create_args_t, test_add)

int main()
{
    BENCHMARKS_RUN(add);
}