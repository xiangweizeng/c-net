
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"


#undef Min
#undef Max
#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))


FUNCTION_IRAM static int inner_product_forward_impl(
        inner_product_t *inner_product,
        tensor_t *bottom_tensor,
        tensor_t *top_tensor,
        option_t *opt) {

    inner_product_config_t config = inner_product->config;

    int16_t* out_ptr = (int16_t*) top_tensor->data;
    int16_t *w0 = inner_product->weights.data;
    int32_t *bias = config.bias_term ? inner_product->bias.data : NULL;
    float *requantize_data = inner_product->requantize.data;

    int32_t out_max = config.max;
    int32_t out_min = config.min;
    int output_num = config.filers_size[3];
    int requantize_num = config.requantize_num;
    int32_t input_num = config.filers_size[2];
    fixed_mul_t leaky = get_fixed_mul(config.leaky);

    for(int p = 0; p < output_num; p ++){
        register int64_t sum0 = 0;
        register int32_t b0 = bias ? bias[p] : 0;
        const int16_t *v = (int16_t *) bottom_tensor->data;
        float output_scale = requantize_num > 1 ? requantize_data[p] : requantize_data[0];
        fixed_mul_t requantize = get_fixed_mul(output_scale);

        register int32_t compute_grid = input_num >> 2;
        register int32_t remain = input_num & 0b11;

        while (compute_grid -- > 0){
            sum0 += v[0] * w0[0];
            sum0 += v[1] * w0[1];
            sum0 += v[2] * w0[2];
            sum0 += v[3] * w0[3];

            v += 4;
            w0 += 4;
        }

        for (int i = 0; i < remain; i++) {
            int32_t value = v[i];
            sum0 += value * w0[i];
        }
        w0 += remain;

        sum0 = REQUANTIZE_BIAS(sum0, requantize, b0);
        sum0 = sum0 > 0 ? sum0 : MULTIPLY_FIDED(sum0, leaky);
        out_ptr[p] = CLIP_INT16(sum0, out_max, out_min);
    }
    return CNET_STATUS_SUCCESS;
}

FUNCTION_IRAM static int inner_product_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    inner_product_t *inner_product = (inner_product_t *) operation->base;
    tensor_t *bottom_tensor = &bottom_tensors->data[0].data;
    tensor_t *top_tensor = &top_tensors->data[0].data;

    return inner_product_forward_impl(inner_product, bottom_tensor, top_tensor, opt);
}

IMPL_OPERATION_CREATOR(inner_product) {
    operation_t *inner_product = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == inner_product) {
        printf("malloc for inner_product failed\n");
        return NULL;
    }

    operation_basic_info_setup(inner_product);
    inner_product->forward = inner_product_forward;
    return (operation_ptr) inner_product;
}
