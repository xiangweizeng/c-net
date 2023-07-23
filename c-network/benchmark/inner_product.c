/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include <allocator.h>
#include <stdint.h>
#include <quantize16.h>
#include "inner_product.h"

#undef Min
#undef Max
#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))

FUNCTION_IRAM void inner_product(inner_product_context_t *context){
   
    const int16_t *w =  context->weight_data;
    int16_t *out_ptr = (int16_t *)context->out_ptr;

    int32_t out_max = context->config.max;
    int32_t out_min = context->config.min;
    fixed_mul_t requantize = context->config.requantize;
    fixed_mul_t leaky = context->config.leaky;
    int32_t in_size = context->config.filers_size[2];
    int32_t out_size = context->config.filers_size[3];

    for(int q = 0; q < out_size; q++){
        register int32_t compute_grid = in_size >> 2;
        register int32_t remain = in_size & 0b11;
        register int64_t sum0 = 0;
        register int32_t b0 = context->config.bias_term ? context->bias_data[q] : 0;
        const int16_t *v = context->input_ptr;

        while (compute_grid-- > 0)
        {
            sum0 += v[0] * w[0];
            sum0 += v[1] * w[1];
            sum0 += v[2] * w[2];
            sum0 += v[3] * w[3];

            v += 4;
            w += 4;
        }

        for (int i = 0; i < remain; i++)
        {
            int32_t value = v[i];
            sum0 += value * w[i];
        }

        w += remain;
        sum0 = REQUANTIZE_BIAS(sum0, requantize, b0);
        sum0 = sum0 > 0 ? sum0 : MULTIPLY_FIDED(sum0, leaky);
        out_ptr[q] = CLIP_INT16(sum0, out_max, out_min);
    }
}
