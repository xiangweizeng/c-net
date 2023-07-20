/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "inner_product_benchmark.h"
#include "allocator.h"

BENCHMARKS(inner_product){
        {.name = "inner_product-100-100-1",
                .args = {
                        .config = {
                                .type = 0,
                                .filers_size = {1, 1, 100, 100},
                                .bias_term = 1,
                                .requantize = {1, 1},
                                .leaky = {0, 0},
                                .max = INT16_MAX,
                                .min = INT16_MIN
                        }
                }
        },
        {.name = "inner_product-1000-100-1",
                .args = {
                        .config = {
                                .type = 0,
                                .filers_size = {1, 1, 1000, 100},
                                .bias_term = 1,
                                .requantize = {1, 1},
                                .leaky = {0, 0},
                                .max = INT16_MAX,
                                .min = INT16_MIN
                        }
                }
        },
        {.name = "inner_product-1000-1000-1",
                .args = {
                        .config = {
                                .type = 0,
                                .filers_size = {1, 1, 1000, 1000},
                                .bias_term = 1,
                                .requantize = {1, 1},
                                .leaky = {0, 0},
                                .max = INT16_MAX,
                                .min = INT16_MIN
                        }
                }
        },
        {.name = "inner_product-10000-1000-1",
                .args = {
                        .config = {
                                .type = 0,
                                .filers_size = {1, 1, 10000, 1000},
                                .bias_term = 1,
                                .requantize = {1, 1},
                                .leaky = {0, 0},
                                .max = INT16_MAX,
                                .min = INT16_MIN
                        }
                }
        },
        {.name = "inner_product-100000-100000-1",
                .args = {
                        .config = {
                                .type = 0,
                                .filers_size = {1, 1, 100000, 1000},
                                .bias_term = 1,
                                .requantize = {1, 1},
                                .leaky = {0, 0},
                                .max = INT16_MAX,
                                .min = INT16_MIN
                        }
                }
        },
};

static benchmark_status_t inner_product_benchmark(inner_product_context_t *args){
    inner_product(args);
    return BENCH_MARK_SUCCESS;
}

static benchmark_status_t inner_product_setup(inner_product_context_t *args){
    args->bias_data = fast_malloc(args->config.filers_size[3] * 2);
    args->weight_data = fast_malloc(args->config.filers_size[3] * args->config.filers_size[2] * 2);
    args->input_ptr = fast_malloc(args->config.filers_size[2] * 2);
    args->out_ptr = fast_malloc(args->config.filers_size[3] * 2);
    return BENCH_MARK_SUCCESS;
}

static benchmark_status_t inner_product_teardown(inner_product_context_t *args){
    if(args->bias_data != NULL){
        fast_free(args->bias_data);
        args->bias_data = NULL;
    }

    if(args->weight_data != NULL){
        fast_free(args->weight_data);
        args->weight_data = NULL;
    }

    if(args->input_ptr != NULL){
        fast_free(args->input_ptr);
        args->input_ptr = NULL;
    }

    if(args->out_ptr != NULL){
        fast_free(args->out_ptr);
        args->out_ptr = NULL;
    }

    return BENCH_MARK_SUCCESS;
}

BENCHMARK_IMPL(inner_product, inner_product_context_t, inner_product_benchmark)

void benchmark_inner_product()
{
    BENCHMARK_SET_SETUP(inner_product, inner_product_setup);
    BENCHMARK_SET_TEAR_DOWN(inner_product, inner_product_teardown);
    BENCHMARKS_RUN(inner_product);
}

int main(){
    benchmark_inner_product();
}
