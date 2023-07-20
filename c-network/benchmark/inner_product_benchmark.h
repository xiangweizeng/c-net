/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef NETWORK_INNER_PRODUCT_BENCHMARK_H
#define NETWORK_INNER_PRODUCT_BENCHMARK_H

#include "benchmark_function.h"
#include "inner_product.h"

BENCHMARK_DECLARE(inner_product, inner_product_context_t)

void benchmark_inner_product();

#endif //NETWORK_INNER_PRODUCT_BENCHMARK_H
