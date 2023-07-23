/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef BENCHMARK_INNER_PRODUCT_H
#define BENCHMARK_INNER_PRODUCT_H

#include <operation_config.h>

typedef struct inner_product_context
{
    inner_product_config_t config;
    int16_t *weight_data;
    int16_t *bias_data;
    int16_t *input_ptr;
    int16_t *out_ptr;
} inner_product_context_t;

FUNCTION_IRAM void inner_product(inner_product_context_t *context);

#endif //BENCHMARK_INNER_PRODUCT_H
