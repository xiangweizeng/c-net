/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/


#include "operation.h"
#include "operation_config.h"
#include "quantize_s8.h"

#include <stdio.h>

typedef struct convolution_context {
    convolution_config_t config;
    tensor_t* bottom;
    tensor_t* top;
    const int8_t* weight_data;
    const int32_t* bias_data;
    const float* requantize_data;
    int out_w;
    int out_h;
}convolution_context_t;

FUNCTION_IRAM int dotprod(const int8_t *src1, const int8_t *src2, int32_t *dest, int len, int8_t shift){
    int32_t inner_dot = 0;

    for(int i = 0; i < len; i++){
        inner_dot += src1[i] * src2[i];
    }
    *dest = inner_dot;
    return 0;
}

FUNCTION_IRAM static void int8_conv(convolution_context_t* context) {

    const float *requantize_data = context->requantize_data;
    fixed_mul_t leaky = get_fixed_mul(context->config.leaky);

    int32_t out_max = context->config.max;
    int32_t out_min = context->config.min;
    int requantize_num = context->config.requantize_num;

    int input_w = context->bottom->d1;
    int input_h = context->bottom->d2;

    int input_c = context->config.filers_size[3];
    int output_c = context->config.filers_size[0];

    int kernel_2d_size = context->config.filers_size[2] * context->config.filers_size[1];
    int kernel_3d_size = kernel_2d_size * input_c;

    int stride_h = context->config.stride_h;
    int stride_w = context->config.stride_w;
    int pad_top = context->config.pad_top;
    int pad_left = context->config.pad_left;

    register int dilation_w = context->config.dilation_w;
    register int dilation_h = context->config.dilation_h;

    register int kernel_h = context->config.filers_size[1];
    register int kernel_w = context->config.filers_size[2];
    int8_t pad_vale = context->config.pad_value;

    tensor_t cache_row = tensor_create_default();
    tensor_create_1d(&cache_row, kernel_3d_size , 1, 0);

    int32_t oy, ox, kx, ky;
    for (oy = 0; oy < context->out_h; oy++)
    {
        for (ox = 0; ox < context->out_w; ox++)
        {
            const int32_t in_y_origin = (oy * stride_h) - pad_top;
            const int32_t in_x_origin = (ox * stride_w) - pad_left;

            int8_t *rows = cache_row.data;
            for (ky = 0; ky < kernel_h; ky++)
            {
                for (kx = 0; kx < kernel_w; kx++)
                {
                    const int32_t in_y = in_y_origin + dilation_h * ky;
                    const int32_t in_x = in_x_origin + dilation_w * kx;

                    const int8_t* in_c_p = (int8_t*)context->bottom->data + (in_y * input_w + in_x) * input_c;
                    if(in_x < 0 || in_x >= input_w || in_y < 0 || in_y >= input_h){
                        for(int p_c = 0; p_c < input_c; p_c ++){
                            rows[p_c] = pad_vale;
                        }
                        rows += input_c;
                    } else {
                        memcpy(rows, in_c_p, sizeof(int8_t) * input_c);
                        rows += input_c;
                    }
                }
            }

            int8_t* out_ptr = (int8_t*)context->top->data + (oy * context->out_w + ox) * output_c;
            for (size_t p = 0; p < output_c; p++)
            {
                int32_t bias_value = context->config.bias_term ? context->bias_data[p] : 0;
                float output_scale = requantize_num > 1 ? requantize_data[p] : requantize_data[0];
                fixed_mul_t requantize = get_fixed_mul(output_scale);
                const int8_t *w0 = context->weight_data + kernel_3d_size * p;

                int32_t sum = 0;
                dotprod(cache_row.data, w0, &sum, kernel_3d_size, 0);

                int32_t sum_i32 = REQUANTIZE_BIAS(sum, requantize, bias_value);
                sum_i32 = sum_i32 > 0 ? sum_i32 : MULTIPLY_FIDED(sum_i32, leaky);
                out_ptr[p] = CLIP_INT8(sum_i32, out_max, out_min);
            }
        }
    }

    tensor_release(&cache_row);
}

FUNCTION_IRAM static void group_conv(convolution_context_t* context) {

    const float *requantize_data = context->requantize_data;
    fixed_mul_t leaky = get_fixed_mul(context->config.leaky);

    int32_t out_max = context->config.max;
    int32_t out_min = context->config.min;
    int requantize_num = context->config.requantize_num;

    int input_w = context->bottom->d1;
    int input_h = context->bottom->d2;
    int input_c = context->config.filers_size[3];
    int output_c = context->config.filers_size[0];

    int input_group_size = context->config.filers_size[3] / context->config.group;
    int output_group_size = context->config.filers_size[0] / context->config.group;

    int kernel_2d_size = context->config.filers_size[2] * context->config.filers_size[1];
    int kernel_3d_size = kernel_2d_size * input_group_size;
    int kernel_group_size = kernel_3d_size * output_group_size;

    int stride_h = context->config.stride_h;
    int stride_w = context->config.stride_w;
    int pad_top = context->config.pad_top;
    int pad_left = context->config.pad_left;
    int8_t pad_vale = context->config.pad_value;

    register int dilation_w = context->config.dilation_w;
    register int dilation_h = context->config.dilation_h;

    register int kernel_h = context->config.filers_size[1];
    register int kernel_w = context->config.filers_size[2];

    tensor_t cache_row = tensor_create_default();
    tensor_create_1d(&cache_row, kernel_3d_size , 2, 0);
    const int8_t* input = (int8_t*)context->bottom->data;

    int32_t oy, ox, kx, ky;
    for (oy = 0; oy < context->out_h; oy++)
    {
        for (ox = 0; ox < context->out_w; ox++)
        {
            const int32_t in_y_origin = (oy * stride_h) - pad_top;
            const int32_t in_x_origin = (ox * stride_w) - pad_left;

            for(int32_t g = 0; g < context->config.group; g ++){
                int8_t* weight_data = (int8_t*)context->weight_data + kernel_group_size * g;
                int8_t* rows = cache_row.data;
                for (ky = 0; ky < kernel_h; ky++)
                {
                    for (kx = 0; kx < kernel_w; kx++)
                    {
                        const int32_t in_y = in_y_origin + dilation_h * ky;
                        const int32_t in_x = in_x_origin + dilation_w * kx;

                        if(in_x < 0 || in_x >= input_w || in_y < 0 || in_y >= input_h){
                            for(int p_c = 0; p_c < input_group_size; p_c ++){
                                rows[p_c] = pad_vale;
                            }
                            rows += input_group_size;
                        } else {
                            int offset = (in_y * input_w + in_x) * input_c + g * input_group_size;
                            for(int p_c = 0; p_c < input_group_size; p_c ++){
                                rows[p_c] = input[offset + p_c];
                            }
                            rows += input_group_size;
                        }
                    }
                }

                int8_t* out_ptr = (int8_t*)context->top->data + (oy * context->out_w + ox) * output_c + g * output_group_size;
                for (int p = 0; p < output_group_size; p++) {

                    const int8_t* w = weight_data + kernel_3d_size * p;
                    int32_t c_offset = g * output_group_size + p;
                    int32_t bias_value = context->config.bias_term ? context->bias_data[c_offset] : 0;
                    float output_scale = requantize_num > 1 ? requantize_data[c_offset] : requantize_data[0];
                    fixed_mul_t requantize = get_fixed_mul(output_scale);

                    int32_t sum = 0;
                    dotprod(cache_row.data, w, &sum, kernel_3d_size, 0);

                    int32_t sum_i32 = REQUANTIZE_BIAS(sum, requantize, bias_value);
                    sum_i32 = sum_i32 > 0 ? sum_i32 : MULTIPLY_FIDED(sum_i32, leaky);
                    out_ptr[p] = CLIP_INT8(sum_i32, out_max, out_min);
                }
            }
        }
    }

    tensor_release(&cache_row);
}

FUNCTION_IRAM static int convolution_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    convolution_t *convolution = (convolution_t *) operation->base;
    tensor_t *bottom_tensor = &bottom_tensors->data[0].data;
    tensor_t *top_tensor = &top_tensors->data[0].data;

    /// prepare context
    convolution_context_t int8_context = {
            convolution->config,
            bottom_tensor,
            top_tensor,
            convolution->filters.data,
            convolution->bias.data,
            convolution->requantize.data,
            top_tensor->d1,
            top_tensor->d2,
    };

    /// run kernel
    if (convolution->config.group == 1) {
        int8_conv(&int8_context);
    }
    else {
        group_conv(&int8_context);
    }

    return CNET_STATUS_SUCCESS;
}

IMPL_OPERATION_CREATOR(convolution) {
    operation_t *convolution = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == convolution) {
        printf("malloc for float-impl failed\n");
        return NULL;
    }

    operation_basic_info_setup(convolution);
    convolution->forward = convolution_forward;
    return (operation_ptr) convolution;
}
