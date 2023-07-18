/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "operation.h"
#include "operation_config.h"
#include "quantize16.h"

typedef struct conv_int16_context {
    tensor_t* bottom;
    tensor_t* top;
    const int16_t* weight_data;
    const int16_t* bias_data;
    convolution_config_t* config;
    int out_w;
    int out_h;
}conv_int16_context_t;

FUNCTION_IRAM static void int16_conv_thread(conv_int16_context_t* context, size_t p) {

    fixed_mul_t requantize = context->config->requantize;
    fixed_mul_t leaky = context->config->leaky;

    int32_t bias_value = context->config->bias_term ? context->bias_data[p] : 0;

    int32_t out_max = context->config->max;
    int32_t out_min = context->config->min;

    int input_w = context->bottom->d0;
    int input_h = context->bottom->d1;
    int input_c = context->bottom->d2;

    int kernel_2d_size = context->config->filers_size[2] * context->config->filers_size[3];
    int kernel_3d_size = kernel_2d_size * input_c;

    int stride_h = context->config->stride_h;
    int stride_w = context->config->stride_w;
    int pad_top = context->config->pad_top;
    int pad_left = context->config->pad_left;

    register int dilation_w = context->config->dilation_w;
    register int dilation_h = context->config->dilation_h;

    register int kernel_h = context->config->filers_size[2];
    register int kernel_w = context->config->filers_size[3];
    int16_t pad_vale = context->config->pad_value;

    int16_t* out_ptr = (int16_t*)tensor_d2(context->top, p).data;
    int32_t oy, ox, ic, kx, ky;
    for (oy = 0; oy < context->out_h; oy++)
    {
        for (ox = 0; ox < context->out_w; ox++)
        {
            const int32_t in_y_origin = (oy * stride_h) - pad_top;
            const int32_t in_x_origin = (ox * stride_w) - pad_left;

            int64_t sum = 0;
            const int16_t* k_ptr = context->weight_data + kernel_3d_size * p;
            for (ic = 0; ic < input_c; ic++)
            {
                tensor_t channel = tensor_d2(context->bottom, ic);
                const int16_t* in_c_p = (int16_t*)channel.data;
                const int16_t* w_ic_p = k_ptr + (size_t)ic * kernel_2d_size;

                for (ky = 0; ky < kernel_h; ky++)
                {
                    for (kx = 0; kx < kernel_w; kx++)
                    {
                        const int32_t in_y = in_y_origin + dilation_h * ky;
                        const int32_t in_x = in_x_origin + dilation_w * kx;

                        const int16_t w = w_ic_p[ky * kernel_w + kx];
                        const int16_t in_v = (in_x < 0 || in_x >= input_w || in_y < 0 || in_y >= input_h)
                                ? pad_vale : in_c_p[in_y * input_w + in_x];

                        sum += in_v * w;
                    }
                }
            }

            int32_t sum_i32 = REQUANTIZE_BIAS(sum, requantize, bias_value);
            sum_i32 = sum_i32 > 0 ? sum_i32 : MULTIPLY_FIDED(sum_i32, leaky);
            out_ptr[ox] = CLIP_INT16(sum_i32, out_max, out_min);
        }

        out_ptr += context->out_w;
    }
}

FUNCTION_IRAM static void int16_group_conv_thread(conv_int16_context_t* context, size_t g) {

    fixed_mul_t requantize = context->config->requantize;
    fixed_mul_t leaky = context->config->leaky;

    int32_t out_max = context->config->max;
    int32_t out_min = context->config->min;

    int input_group_size = context->config->filers_size[1] / context->config->group;
    int output_group_size = context->config->filers_size[0] / context->config->group;

    int input_w = context->bottom->d0;
    int input_h = context->bottom->d1;
    int input_c = output_group_size;

    int kernel_2d_size = context->config->filers_size[2] * context->config->filers_size[3];
    int kernel_3d_size = kernel_2d_size * input_group_size;
    int kernel_group_size = kernel_3d_size * output_group_size;

    int stride_h = context->config->stride_h;
    int stride_w = context->config->stride_w;
    int pad_top = context->config->pad_top;
    int pad_left = context->config->pad_left;

    int16_t pad_vale = context->config->pad_value;
    int16_t* weight_data = (int16_t*)context->weight_data + kernel_group_size * g;

    register int dilation_w = context->config->dilation_w;
    register int dilation_h = context->config->dilation_h;

    register int kernel_h = context->config->filers_size[2];
    register int kernel_w = context->config->filers_size[3];

    int32_t p, oy, ox, ic, kx, ky;
    for (p = 0; p < output_group_size; p++) {
        int16_t* out_ptr = (int16_t*)tensor_d2(context->top, g * output_group_size + p).data;
        int32_t bias_value = context->config->bias_term ? context->bias_data[g * output_group_size + p] : 0;

        for (oy = 0; oy < context->out_h; oy++)
        {
            for (ox = 0; ox < context->out_w; ox++)
            {
                const int32_t in_y_origin = (oy * stride_h) - pad_top;
                const int32_t in_x_origin = (ox * stride_w) - pad_left;

                int64_t sum = 0;

                const int16_t* k_ptr = weight_data + kernel_3d_size * p;
                for (ic = 0; ic < input_c; ic++)
                {
                    tensor_t channel = tensor_d2(context->bottom, g * input_group_size + ic);
                    const int16_t* in_c_p = (int16_t*)channel.data;
                    const int16_t* w_ic_p = k_ptr + (size_t)ic * kernel_2d_size;

                    for (ky = 0; ky < kernel_h; ky++)
                    {
                        for (kx = 0; kx < kernel_w; kx++)
                        {
                            const int32_t in_y = in_y_origin + dilation_h * ky;
                            const int32_t in_x = in_x_origin + dilation_w * kx;

                            const int16_t w = w_ic_p[ky * kernel_w + kx];
                            const int16_t in_v = (in_x < 0 || in_x >= input_w || in_y < 0 || in_y >= input_h) ?
                                    pad_vale : in_c_p[in_y * input_w + in_x];

                            sum += in_v * w;
                        }
                    }
                }

                int32_t sum_i32 = REQUANTIZE_BIAS(sum, requantize, bias_value);
                sum_i32 = sum_i32 > 0 ? sum_i32 : MULTIPLY_FIDED(sum_i32, leaky);
                out_ptr[ox] = CLIP_INT16(sum_i32, out_max, out_min);
            }

            out_ptr += context->out_w;
        }
    }
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
    const int16_t* weight_data = (const int16_t*)convolution->filters.data.data;
    const int16_t* bias_data = (const int16_t*)convolution->bias.data.data;
    conv_int16_context_t int16_context = {
            bottom_tensor,
            top_tensor,
            weight_data,
            bias_data,
            &convolution->config,
            top_tensor->d0,
            top_tensor->d1,
    };

    if(convolution->config.filers_size[0] != top_tensor->d2 || convolution->config.filers_size[1] != bottom_tensor->d2){
        printf("Param error conv\n");
        return CNET_STATUS_FAILED;
    }

    /// run kernel
    if (convolution->config.group == 1) {
        PARALLELIZE_1D(int16_conv_thread, int16_context, convolution->config.filers_size[0]);
    }
    else {
        if(convolution->config.filers_size[0] != convolution->config.filers_size[1]){
            printf("Param error conv\n");
            return CNET_STATUS_FAILED;
        }
        PARALLELIZE_1D(int16_group_conv_thread, int16_context, convolution->config.group);
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
