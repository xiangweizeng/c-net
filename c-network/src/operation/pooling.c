
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


FUNCTION_IRAM static int global_pooling_forward(pooling_t *pooling, tensor_t *bottom_tensor, tensor_t *top_tensor, option_t *opt) {
    int channels = bottom_tensor->d0;
    int input_w = bottom_tensor->d1;
    int input_h = bottom_tensor->d2;

    int8_t *output = top_tensor->data;
    int8_t *input = bottom_tensor->data;

    if (pooling->config.pooling_type == pool_method_max_type) {
        for(int c = 0; c < channels; c++){
            int32_t max = 0;
            for(int h_ = 0; h_ < input_h; h_ ++){
                for(int w_ = 0; w_ < input_w; w_ ++){
                    const int points_input = (h_ * input_w + w_) * channels;
                    max = Max(input[points_input + c], max);
                }
            }

            output[c] = max;
        }
    } else if (pooling->config.pooling_type == pool_method_avg_type) {
        int size = input_w * input_h;
        for(int c = 0; c < channels; c++){
            int32_t sum = 0;
            for(int h_ = 0; h_ < input_h; h_ ++){
                for(int w_ = 0; w_ < input_w; w_ ++){
                    const int points_input = (h_ * input_w + w_) * channels;
                    sum += input[points_input + c];
                }
            }

            sum = sum / size;
            output[c] = sum;
        }
    }

    return CNET_STATUS_SUCCESS;
}


typedef struct normal_pooling_context_t{
    pooling_config_t config;
    tensor_t *bottom_tensor;
    tensor_t *top_tensor;
    int out_w;
    int out_h;
    int channels;
}normal_pooling_context_t;

FUNCTION_IRAM static void normal_pool_max(normal_pooling_context_t *context) {
    int8_t *out_ptr = (int8_t *)context->top_tensor->data;
    const int8_t *in_ptr = (const int8_t *) context->bottom_tensor->data;

    int input_w = context->bottom_tensor->d1;
    int input_h = context->bottom_tensor->d2;
    int channels = context->channels;

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

            int8_t *points_output = out_ptr + ( oy * context->out_w + ox) * channels;
            for(int32_t oc = 0; oc < channels; oc ++){
                int8_t value = INT8_MIN;
                for (int32_t ky = filter_y_start; ky < filter_y_end; ky++)
                {
                    for (int32_t kx = filter_x_start; kx < filter_x_end; kx++)
                    {
                        const int32_t in_y = in_y_origin + ky;
                        const int32_t in_x = in_x_origin + kx;
                        const int points_input = (in_y * input_w + in_x) * channels;

                        const int8_t in_v = in_ptr[points_input + oc];
                        value = Max(value, in_v);
                    }
                }
                points_output[oc] = value;
            }
        }
    }
}

FUNCTION_IRAM static void normal_pool_ave(normal_pooling_context_t *context) {
    int8_t *out_ptr = (int8_t *)context->top_tensor->data;
    const int8_t *in_ptr = (const int8_t *) context->bottom_tensor->data;

    int input_w = context->bottom_tensor->d1;
    int input_h = context->bottom_tensor->d2;
    int channels = context->channels;
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

            int8_t *points_output = out_ptr + ( oy * context->out_w + ox) * channels;
            for(int32_t oc = 0; oc < channels; oc ++) {

                int32_t value = 0;
                int kernel_count = 0;
                for (int32_t ky = filter_y_start; ky < filter_y_end; ky++)
                {
                    for (int32_t kx = filter_x_start; kx < filter_x_end; kx++)
                    {
                        const int32_t in_y = in_y_origin + ky;
                        const int32_t in_x = in_x_origin + kx;
                        const int points_input = (in_y * input_w + in_x) * channels;

                        kernel_count ++;
                        const int8_t in_v = in_ptr[points_input + oc];
                        value += in_v;
                    }
                }

                kernel_count = context->config.count_include_pad ? all_kernel_count : kernel_count;
                value = kernel_count > 0 ? value / kernel_count : 0;

                points_output[oc] = value;
            }
        }
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
            pooling->config,
            &bottom->data,
            &top->data,
            top->data.d1,
            top->data.d2,
            top->data.d0
    };

    if(pool_method_max_type == pooling->config.pooling_type){
        normal_pool_max(&context);
    }else {
        normal_pool_ave(&context);
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
