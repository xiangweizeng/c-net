
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

typedef struct global_polling_context_t{
    tensor_t *bottom_tensor;
    tensor_t *top_tensor;
    pooling_config_t config;
    int size;
}global_polling_context_t;

FUNCTION_IRAM static void global_pooling_max_thread(global_polling_context_t *context, int q) {
    tensor_t channel = tensor_d2(context->bottom_tensor, q);
    int16_t *ptr = (int16_t *) channel.data;

    int8_t max = ptr[0];
    for (int i = 1; i < context->size; i++) {
        max = Max(max, ptr[i]);
    }

    ((int16_t *) context->top_tensor->data)[q] = MULTIPLY_FIDED(max, context->config.requantize);
}

FUNCTION_IRAM void global_pooling_ave_thread(global_polling_context_t *context, int q) {
    tensor_t channel = tensor_d2(context->bottom_tensor, q);
    const int16_t *ptr = (const int16_t *) channel.data;
    int size = context->size;

    int32_t sum = 0.f;
    for (int i = 0; i < size; i++) {
        sum += ptr[i];
    }

    sum = sum / size;
    ((int16_t *) context->top_tensor->data)[q] = MULTIPLY_FIDED(sum, context->config.requantize);
}

FUNCTION_IRAM static int global_pooling_forward(pooling_t *pooling, tensor_t *bottom_tensor, tensor_t *top_tensor, option_t *opt) {
    int size = bottom_tensor->d0 * bottom_tensor->d1;
    int channels = bottom_tensor->d2;

    global_polling_context_t context = {
            .bottom_tensor = bottom_tensor,
            .top_tensor = top_tensor,
            .config = pooling->config,
            .size = size
    };
    if (pooling->config.pooling_type == pool_method_max_type) {
        PARALLELIZE_1D(global_pooling_max_thread, context, channels);
    } else if (pooling->config.pooling_type == pool_method_avg_type) {
        PARALLELIZE_1D(global_pooling_ave_thread, context, channels);
    }

    return CNET_STATUS_SUCCESS;
}


typedef struct normal_pooling_context_t{
    tensor_t *bottom_tensor;
    tensor_t *top_tensor;
    pooling_config_t config;
    int out_w;
    int out_h;
}normal_pooling_context_t;

FUNCTION_IRAM static void normal_pool_max_thread(normal_pooling_context_t *context, int q) {
    tensor_t m = tensor_d2(context->bottom_tensor, q);
    tensor_t out_m = tensor_d2(context->top_tensor, q);
    int16_t *out_ptr = (int16_t *)out_m.data;
    const int16_t *in_ptr = (const int16_t *) m.data;

    int input_w = context->bottom_tensor->d0;
    int input_h = context->bottom_tensor->d1;

    for (int32_t oy = 0; oy < context->out_h; oy++)
    {
        for (int32_t ox = 0; ox < context->out_w; ox++)
        {
            const int32_t in_y_origin = (oy * context->config.stride_h) - context->config.pad_top;
            const int32_t in_x_origin = (ox * context->config.stride_w) - context->config.pad_left;
            const int32_t filter_y_start = Max(0, -in_y_origin);
            const int32_t filter_y_end = Min(context->config.kernel_h, input_h - in_y_origin);
            const int32_t filter_x_start = Max(0, -in_x_origin );
            const int32_t filter_x_end = Min(context->config.kernel_w, input_w - in_x_origin );
            int32_t value = INT16_MIN;

            for (int32_t ky = filter_y_start; ky < filter_y_end; ky++)
            {
                for (int32_t kx = filter_x_start; kx < filter_x_end; kx++)
                {
                    const int32_t in_y = in_y_origin + ky;
                    const int32_t in_x = in_x_origin + kx;

                    const int16_t in_v = in_ptr[in_y * input_w + in_x];
                    value = Max(value, in_v);
                }
            }

            value =  MULTIPLY_FIDED(value, context->config.requantize);
            out_ptr[ox] = CLIP_INT16(value, INT16_MAX, INT16_MIN);
        }

        out_ptr += context->out_w;
    }
}

FUNCTION_IRAM static void normal_pool_ave_thread(normal_pooling_context_t *context, int q) {
    tensor_t m = tensor_d2(context->bottom_tensor, q);
    tensor_t out_m = tensor_d2(context->top_tensor, q);
    int16_t *out_ptr = (int16_t *) out_m.data;
    const int16_t *in_ptr = (const int16_t *) m.data;

    int input_w = context->bottom_tensor->d0;
    int input_h = context->bottom_tensor->d1;
    int all_kernel_count = context->config.kernel_h*context->config.kernel_w;

    for (int32_t oy = 0; oy < context->out_h; oy++)
    {
        for (int32_t ox = 0; ox < context->out_w; ox++)
        {
            const int32_t in_y_origin = (oy * context->config.stride_h) - context->config.pad_top;
            const int32_t in_x_origin = (ox * context->config.stride_w) - context->config.pad_left;
            const int32_t filter_y_start = Max(0, -in_y_origin);
            const int32_t filter_y_end = Min(context->config.kernel_h, input_h - in_y_origin);
            const int32_t filter_x_start = Max(0, -in_x_origin );
            const int32_t filter_x_end = Min(context->config.kernel_w, input_w - in_x_origin );
            int32_t value = 0;
            int kernel_count = 0;
            for (int32_t ky = filter_y_start; ky < filter_y_end; ky++)
            {
                for (int32_t kx = filter_x_start; kx < filter_x_end; kx++)
                {
                    const int32_t in_y = in_y_origin + ky;
                    const int32_t in_x = in_x_origin + kx;

                    kernel_count ++;
                    const int16_t in_v = in_ptr[in_y * input_w + in_x];
                    value += in_v;
                }
            }
            kernel_count = context->config.count_include_pad ? all_kernel_count : kernel_count;
            value = kernel_count > 0 ? value / kernel_count : 0;

            value =  MULTIPLY_FIDED(value, context->config.requantize);
            out_ptr[ox] = CLIP_INT16(value, INT16_MAX, INT16_MIN);
        }

        out_ptr += context->out_w;
    }

}

FUNCTION_IRAM static int normal_pooling_forward(
        pooling_t *pooling,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    blob_container_t *bottom = &bottom_tensors->data[0];
    blob_container_t *top = &top_tensors->data[0];

    normal_pooling_context_t context = {
            &bottom->data,
            &top->data,
            pooling->config,
            top->data.d0,
            top->data.d1
    };

    if(pool_method_max_type == pooling->config.pooling_type){
        PARALLELIZE_1D(normal_pool_max_thread, context, top->data.d2);
    }else {
        PARALLELIZE_1D(normal_pool_ave_thread, context, top->data.d2);
    }

    return CNET_STATUS_SUCCESS;
}


FUNCTION_IRAM static int pooling_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    pooling_t *pooling = (pooling_t *) operation->base;
    if(pooling->config.global_pooling){
        tensor_t *bottom_tensor = &bottom_tensors->data[0].data;
        tensor_t *top_tensor = &top_tensors->data[0].data;
        return global_pooling_forward(pooling, bottom_tensor, top_tensor, opt);
    } else{
        return normal_pooling_forward(pooling, bottom_tensors, top_tensors, opt);
    }
}

IMPL_OPERATION_CREATOR(pooling) {
    operation_t *pooling = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == pooling) {
        printf("malloc for pooling failed\n");
        return NULL;
    }

    operation_basic_info_setup(pooling);
    pooling->forward = pooling_forward;
    return (operation_ptr) pooling;
}
