
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

typedef struct inner_product_context_t {
    tensor_t *bottom_tensor;
    inner_product_config_t config;
    int16_t *weight_data;
    int16_t *bias_data;
    int16_t *out_ptr;
} inner_product_context_t;

FUNCTION_IRAM void inner_product_requantize_tile(inner_product_context_t *context, size_t p, size_t tile) {

    int16_t *weight_data = context->weight_data;
    tensor_t *bottom_tensor = context->bottom_tensor;
    int16_t* out_ptr = (int16_t*)context->out_ptr;

    int32_t size = tensor_total(bottom_tensor);
    const int16_t *w0 = weight_data + size * p;
    const int16_t *w1 = w0 + size;
    const int16_t *w2 = w1 + size;
    const int16_t *w3 = w2 + size;
    const int16_t *v = (int16_t *) bottom_tensor->data;

    int32_t out_max = context->config.max;
    int32_t out_min = context->config.min;
    fixed_mul_t requantize = context->config.requantize;
    fixed_mul_t leaky = context->config.leaky;

    register int32_t compute_grid = size >> 2;
    register int32_t remain = size & 0b11;
    register int32_t grid_v[4] = {0};

    if(tile == 4){
        register int64_t sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
        register int32_t b0 = context->config.bias_term ? context->bias_data[p] : 0;
        register int32_t b1 = context->config.bias_term ? context->bias_data[p+1] : 0;
        register int32_t b2 = context->config.bias_term ? context->bias_data[p+2] : 0;
        register int32_t b3 = context->config.bias_term ? context->bias_data[p+3] : 0;
        while (compute_grid -- > 0){
            grid_v[0] = v[0];
            grid_v[1] = v[1];
            grid_v[2] = v[2];
            grid_v[3] = v[3];

            sum0 += grid_v[0] * w0[0];
            sum1 += grid_v[0] * w1[0];
            sum2 += grid_v[0] * w2[0];
            sum3 += grid_v[0] * w3[0];

            sum0 += grid_v[1] * w0[1];
            sum1 += grid_v[1] * w1[1];
            sum2 += grid_v[1] * w2[1];
            sum3 += grid_v[1] * w3[1];

            sum0 += grid_v[2] * w0[2];
            sum1 += grid_v[2] * w1[2];
            sum2 += grid_v[2] * w2[2];
            sum3 += grid_v[2] * w3[2];

            sum0 += grid_v[3] * w0[3];
            sum1 += grid_v[3] * w1[3];
            sum2 += grid_v[3] * w2[3];
            sum3 += grid_v[3] * w3[3];

            v += 4;
            w0 += 4;
            w1 += 4;
            w2 += 4;
            w3 += 4;
        }

        for (int i = 0; i < remain; i++) {
            int32_t value = v[i];
            sum0 += value * w0[i];
            sum1 += value * w1[i];
            sum2 += value * w2[i];
            sum3 += value * w3[i];
        }

        sum0 = REQUANTIZE_BIAS(sum0, requantize, b0);
        sum1 = REQUANTIZE_BIAS(sum1, requantize, b1);
        sum2 = REQUANTIZE_BIAS(sum2, requantize, b2);
        sum3 = REQUANTIZE_BIAS(sum3, requantize, b3);

        sum0 = sum0 > 0 ? sum0 : MULTIPLY_FIDED(sum0, leaky);
        sum1 = sum1 > 0 ? sum1 : MULTIPLY_FIDED(sum1, leaky);
        sum2 = sum2 > 0 ? sum2 : MULTIPLY_FIDED(sum2, leaky);
        sum3 = sum3 > 0 ? sum3 : MULTIPLY_FIDED(sum3, leaky);

        out_ptr[p] = CLIP_INT16(sum0, out_max, out_min);
        out_ptr[p+1] = CLIP_INT16(sum1, out_max, out_min);
        out_ptr[p+2] = CLIP_INT16(sum2, out_max, out_min);
        out_ptr[p+3] = CLIP_INT16(sum3, out_max, out_min);
    } else if(tile == 3){
        register int64_t sum0 = 0, sum1 = 0, sum2 = 0;
        register int32_t b0 = context->config.bias_term ? context->bias_data[p] : 0;
        register int32_t b1 = context->config.bias_term ? context->bias_data[p+1] : 0;
        register int32_t b2 = context->config.bias_term ? context->bias_data[p+2] : 0;
        while (compute_grid -- > 0){
            grid_v[0] = v[0];
            grid_v[1] = v[1];
            grid_v[2] = v[2];
            grid_v[3] = v[3];

            sum0 += grid_v[0] * w0[0];
            sum1 += grid_v[0] * w1[0];
            sum2 += grid_v[0] * w2[0];

            sum0 += grid_v[1] * w0[1];
            sum1 += grid_v[1] * w1[1];
            sum2 += grid_v[1] * w2[1];

            sum0 += grid_v[2] * w0[2];
            sum1 += grid_v[2] * w1[2];
            sum2 += grid_v[2] * w2[2];

            sum0 += grid_v[3] * w0[3];
            sum1 += grid_v[3] * w1[3];
            sum2 += grid_v[3] * w2[3];

            v += 4;
            w0 += 4;
            w1 += 4;
            w2 += 4;
        }

        for (int i = 0; i < remain; i++) {
            int32_t value = v[i];
            sum0 += value * w0[i];
            sum1 += value * w1[i];
            sum2 += value * w2[i];
        }

        sum0 = REQUANTIZE_BIAS(sum0, requantize, b0);
        sum1 = REQUANTIZE_BIAS(sum1, requantize, b1);
        sum2 = REQUANTIZE_BIAS(sum2, requantize, b2);

        sum0 = sum0 > 0 ? sum0 : MULTIPLY_FIDED(sum0, leaky);
        sum1 = sum1 > 0 ? sum1 : MULTIPLY_FIDED(sum1, leaky);
        sum2 = sum2 > 0 ? sum2 : MULTIPLY_FIDED(sum2, leaky);

        out_ptr[p] = CLIP_INT16(sum0, out_max, out_min);
        out_ptr[p+1] = CLIP_INT16(sum1, out_max, out_min);
        out_ptr[p+2] = CLIP_INT16(sum2, out_max, out_min);
    }else if(tile == 2){
        register int64_t sum0 = 0, sum1 = 0;
        register int32_t b0 = context->config.bias_term ? context->bias_data[p] : 0;
        register int32_t b1 = context->config.bias_term ? context->bias_data[p+1] : 0;
        while (compute_grid -- > 0){
            grid_v[0] = v[0];
            grid_v[1] = v[1];
            grid_v[2] = v[2];
            grid_v[3] = v[3];

            sum0 += grid_v[0] * w0[0];
            sum1 += grid_v[0] * w1[0];

            sum0 += grid_v[1] * w0[1];
            sum1 += grid_v[1] * w1[1];

            sum0 += grid_v[2] * w0[2];
            sum1 += grid_v[2] * w1[2];

            sum0 += grid_v[3] * w0[3];
            sum1 += grid_v[3] * w1[3];

            v += 4;
            w0 += 4;
            w1 += 4;
        }

        for (int i = 0; i < remain; i++) {
            int32_t value = v[i];
            sum0 += value * w0[i];
            sum1 += value * w1[i];
        }

        sum0 = REQUANTIZE_BIAS(sum0, requantize, b0);
        sum1 = REQUANTIZE_BIAS(sum1, requantize, b1);

        sum0 = sum0 > 0 ? sum0 : MULTIPLY_FIDED(sum0, leaky);
        sum1 = sum1 > 0 ? sum1 : MULTIPLY_FIDED(sum1, leaky);

        out_ptr[p] = CLIP_INT16(sum0, out_max, out_min);
        out_ptr[p+1] = CLIP_INT16(sum1, out_max, out_min);
    }else if(tile == 1){
        register int64_t sum0 = 0;
        register int32_t b0 = context->config.bias_term ? context->bias_data[p] : 0;
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

        sum0 = REQUANTIZE_BIAS(sum0, requantize, b0);
        sum0 = sum0 > 0 ? sum0 : MULTIPLY_FIDED(sum0, leaky);
        out_ptr[p] = CLIP_INT16(sum0, out_max, out_min);
    }
}


FUNCTION_IRAM static int inner_product_forward_impl(
        inner_product_t *inner_product,
        tensor_t *bottom_tensor,
        tensor_t *top_tensor,
        option_t *opt) {

    inner_product_context_t context_mul = {
            bottom_tensor,
            inner_product->config,
            inner_product->weights.data.data,
            inner_product->bias.data.data,
            top_tensor->data,
    };

    PARALLELIZE_1D_TILE_1D(inner_product_requantize_tile, context_mul, top_tensor->d0, 4);
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
