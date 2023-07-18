
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"

typedef struct bnll_context{
    float in_scale;
    float out_scale;
    int16_t *output;
}bnll_context_t;

static void bnll_thread_tile(bnll_context_t *context, size_t index, size_t tile)
{
    int16_t *ptr = (int16_t *)context->output + index;
    float in_scale = context->in_scale;
    float out_scale = context->out_scale;
    for (int i = 0; i < tile; i++) {
        float x = ptr[i] / in_scale;
        if (x > 0){
            x = x + log(1.f + exp(-x));
        }
        else{
            x = log(1.f + exp(x));
        }
        ptr[i] = float2int16(x * out_scale);
    }
}

typedef struct clip_context{
    fixed_mul_t requantize;
    int16_t max;
    int16_t min;
    int16_t *output;
}clip_context_t;

static void clip_thread_tile(clip_context_t *context, size_t index, size_t tile)
{
    int16_t *ptr = (int16_t*)context->output + index;
    fixed_mul_t fixed = context->requantize;
    int16_t max = context->max;
    int16_t min = context->max;

    for (int i = 0; i < tile; i++) {
        int32_t v = MULTIPLY_FIDED(ptr[i], fixed);
        v = v > max ? max : v;
        v = v < min ? min : v;
        ptr[i] = v;
    }
}

typedef struct elu_context{
    float in_scale;
    float out_scale;
    float alpha;
    int16_t *output;
}elu_context_t;

static void elu_thread_tile(elu_context_t *context, size_t index, size_t tile)
{
    int16_t *ptr = (int16_t*)context->output + index;
    float in_scale = context->in_scale;
    float out_scale = context->out_scale;
    float alpha = context->alpha;
    for (int i = 0; i < tile; i++) {
        float x = ptr[i] / in_scale;
        if (x < 0.f){
            x = alpha * (exp(x) - 1.f);
        }
        ptr[i] = float2int16(x * out_scale);
    }
}

typedef struct exp_thread_context_t{
    float in_scale;
    float out_scale;
    float value_base;
    float shift;
    float scale;
    int16_t *output;
}exp_thread_context_t;

static void exp1_thread_tile(exp_thread_context_t *context, size_t index, size_t tile)
{
    int16_t *ptr = (int16_t*)context->output + index;
    float in_scale = context->in_scale;
    float out_scale = context->out_scale;
    float scale = context->scale;
    float shift = context->shift;

    for (int i = 0; i < tile; i++) {
        float x = ptr[i] / in_scale;
        x = exp(shift + x * scale);
        ptr[i] = float2int16(x * out_scale);
    }
}

static void exp2_thread_tile(exp_thread_context_t *context, size_t index, size_t tile)
{
    int16_t *ptr = (int16_t*)context->output + index;
    float in_scale = context->in_scale;
    float out_scale = context->out_scale;
    float shift = context->shift;
    float scale = context->scale;

    float value_base = context->value_base;
    for (int i = 0; i < tile; i++) {
        float x = ptr[i] / in_scale;
        ptr[i] = pow(value_base, (shift + x * scale));
        ptr[i] = float2int16(x * out_scale);
    }
}

typedef struct hard_sigmoid_thread_context{
    int32_t lower;
    int32_t upper;
    float alpha;
    float beta;
    float output_scale;
    int16_t *output;
} hard_sigmoid_thread_context_t;

static void hard_sigmoid_thread_tile(hard_sigmoid_thread_context_t *context, size_t index, size_t tile) {
    int16_t *ptr = context->output + index;
    float alpha = context->alpha;
    float beta = context->beta;
    float output_scale = context->output_scale;
    int32_t upper = context->upper;
    int32_t lower = context->lower;

    for (int i = 0; i < tile; i++) {
        int32_t x = ptr[i];
        if (x < lower){
            x = 0;
        }
        else if (x > upper) {
            x = output_scale;
        }
        else {
            x = x * alpha + beta;
        }
        ptr[i] = CLIP_INT16(x, INT16_MAX, INT16_MIN);
    }
}

typedef struct hard_swish_thread_context {
    int32_t lower;
    int32_t upper;
    float alpha;
    float beta;
    float requantize;
    int16_t *output;
} hard_swish_thread_context_t;

static void hard_swish_thread_tile(hard_swish_thread_context_t *context, size_t index, size_t tile) {
    int16_t *ptr = context->output + index;
    float alpha = context->alpha;
    float beta = context->beta;
    float requantize = context->requantize;
    int32_t upper = context->upper;
    int32_t lower = context->lower;

    for (int i = 0; i < tile; i++) {
        int32_t x = ptr[i];
        if (ptr[i] < lower){
            x = 0;
        }
        else if (ptr[i] > upper){
            x = x * requantize;
        }
        else{
            x = alpha * x * x + beta * x;
        }

        ptr[i] = CLIP_INT16(x, INT16_MAX, INT16_MIN);
    }
}

typedef struct log_thread_context_t{
    float in_scale;
    float out_scale;
    float value_base;
    float shift;
    float scale;
    int16_t *output;
}log_thread_context_t;

static void log1_thread_tile(log_thread_context_t *context, size_t index, size_t tile)
{
    int16_t *ptr = (int16_t*)context->output + index;
    float in_scale = context->in_scale;
    float out_scale = context->out_scale;
    float scale = context->scale;
    float shift = context->shift;

    for (int i = 0; i < tile; i++) {
        float x = ptr[i] / in_scale;
        x = log(shift + x * scale);
        ptr[i] = float2int16(x * out_scale);
    }
}

static void log2_thread_tile(log_thread_context_t *context, size_t index, size_t tile)
{
    int16_t *ptr = (int16_t*)context->output + index;
    float in_scale = context->in_scale;
    float out_scale = context->out_scale;
    float scale = context->scale;
    float shift = context->shift;

    float log_base_inv = 1.f / log(context->value_base);
    for (int i = 0; i < tile; i++) {
        float x = ptr[i] / in_scale;
        x = log(shift + x * scale) * log_base_inv;
        ptr[i] = float2int16(x * out_scale);
    }
}


typedef struct power_thread_context_t{
    float in_scale;
    float out_scale;
    float shift;
    float scale;
    float power;
    int16_t *output;
}power_thread_context_t;

static void power_thread_tile(power_thread_context_t *context, size_t index, size_t tile)
{
    int16_t *ptr = (int16_t*)context->output + index;
    float in_scale = context->in_scale;
    float out_scale = context->out_scale;
    float scale = context->scale;
    float shift = context->shift;
    float power = context->power;

    for (int i = 0; i < tile; i++) {
        float x = ptr[i] / in_scale;
        x = pow((shift + x * scale), power);
        ptr[i] = float2int16(x * out_scale);
    }
}

typedef struct relu_context{
    fixed_mul_t requantize;
    fixed_mul_t slope_fixed;
    int16_t *output;
}relu_context_t;


FUNCTION_IRAM static void relu_thread_tile(relu_context_t *context, size_t index, size_t tile) {

    fixed_mul_t slope_fixed = context->slope_fixed;
    fixed_mul_t requantize = context->requantize;
    int16_t *ptr = context->output + index;

    for (int j = 0; j < tile; j++) {
        int32_t v = ptr[j];
        v = v < 0 ? MULTIPLY_FIDED(v, slope_fixed) : MULTIPLY_FIDED(v, requantize);
        ptr[j] = CLIP_INT16(v, INT16_MAX, INT16_MIN);
    }
}


typedef struct selu_thread_context_t{
    float in_scale;
    float out_scale;
    float alphaxlambda;
    float lambda;
    int16_t *output;
}selu_thread_context_t;

static void selu_thread_tile(selu_thread_context_t *context, size_t index, size_t tile)
{
    int16_t *ptr = (int16_t*)context->output + index;
    float in_scale = context->in_scale;
    float out_scale = context->out_scale;
    float alphaxlambda = context->alphaxlambda;
    float lambda = context->lambda;

    for (int i = 0; i < tile; i++) {
        float x = ptr[i] / in_scale;
        if (x < 0.f){
            x = (exp(x) - 1.f) * alphaxlambda;
        }
        else{
            x *= lambda;
        }

        ptr[i] = float2int16(x * out_scale);
    }
}


typedef struct swish_thread_context{
    float in_scale;
    float out_scale;
    int16_t *output;
}swish_thread_context_t;

static void swish_thread_tile(swish_thread_context_t *context, size_t index, size_t tile)
{

    float in_scale = context->in_scale;
    float out_scale = context->out_scale;
    int16_t *ptr = (int16_t*)context->output + index;

    for (int j = 0; j < tile; j++) {
        float x = ptr[j] / in_scale;
        x =  x / (1.f + expf(-x));
        ptr[j] = float2int16(x * out_scale);
    }
}

typedef struct threshold_context{
    int16_t threshold;
    int16_t *output;
}threshold_context_t;

static void threshold_thread_tile(threshold_context_t *context, size_t index, size_t tile)
{
    int16_t *ptr = (int16_t*)context->output + index;
    int16_t threshold = context->threshold;

    for (int i = 0; i < tile; i++) {
        ptr[i] = ptr[i] > threshold ? 1 : 0;
    }
}

FUNCTION_IRAM static int activation_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {


    blob_container_t *bottom = &bottom_tensors->data[0];
    blob_container_t *top = &top_tensors->data[0];

    tensor_t * bottom_top_blob = &top->data;
    size_t  size = tensor_total(bottom_top_blob);

    int thread_number = opt->thread_number > 0 ? opt->thread_number : 1;
    size_t group_size = size / thread_number;

    activation_t *activation = (activation_t*)operation->base;
    switch (activation->config.activate_type) {
        case bnll_activate_type: {
            bnll_context_t context = {
                .out_scale = top->blob->scale,
                .in_scale = bottom->blob->scale,
                .output = bottom_top_blob->data
            };
            PARALLELIZE_1D_TILE_1D(bnll_thread_tile, context, size, group_size);
            return CNET_STATUS_SUCCESS;
        }
        case clip_activate_type:{
            float out_scale = top->blob->scale;
            fixed_mul_t fixed = get_fixed_mul(out_scale / bottom->blob->scale);
            clip_context_t context = {
                    .requantize = fixed,
                    .min = float2int16(activation->config.activate_params[0] * out_scale),
                    .max = float2int16(activation->config.activate_params[1] * out_scale),
                    .output = bottom_top_blob->data,
            };
            PARALLELIZE_1D_TILE_1D(clip_thread_tile, context, size, group_size);
            return CNET_STATUS_SUCCESS;
        }
        case elu_activate_type:{
            elu_context_t context = {
                    .out_scale = top->blob->scale,
                    .in_scale = bottom->blob->scale,
                    .alpha = activation->config.activate_params[0],
                    .output = bottom_top_blob->data,
            };
            PARALLELIZE_1D_TILE_1D(elu_thread_tile, context, size, group_size);
            return CNET_STATUS_SUCCESS;
        }
        case exp_activate_type:{
            exp_thread_context_t context = {
                    .out_scale = top->blob->scale,
                    .in_scale = bottom->blob->scale,
                    .value_base = activation->config.activate_params[0],
                    .scale = activation->config.activate_params[1],
                    .shift = activation->config.activate_params[2],
                    .output = bottom_top_blob->data,
            };

            if (context.value_base == -1.f)
            {
                PARALLELIZE_1D_TILE_1D(exp1_thread_tile, context, size, group_size);
                return CNET_STATUS_SUCCESS;
            }
            else
            {
                PARALLELIZE_1D_TILE_1D(exp2_thread_tile, context, size, group_size);
                return CNET_STATUS_SUCCESS;
            }
        }
        case hard_sigmoid_activate_type:{
            float in_scale = bottom->blob->scale;
            float out_scale = top->blob->scale;
            float alpha = activation->config.activate_params[0];
            float beta = activation->config.activate_params[1];
            float lower = - beta / alpha;
            float upper = (1.f / alpha) + lower;
            hard_sigmoid_thread_context_t context = {
                    .lower = lower * in_scale,
                    .upper = upper * in_scale,
                    .alpha = (alpha * out_scale) / (in_scale),
                    .beta = beta * out_scale,
                    .output_scale = out_scale,
                    .output = top->data.data
            };

            PARALLELIZE_1D_TILE_1D(hard_sigmoid_thread_tile, context, size, group_size);
            return CNET_STATUS_SUCCESS;
        }
        case hard_swish_activate_type:{
            float in_scale = bottom->blob->scale;
            float out_scale = top->blob->scale;
            float requantize = out_scale / in_scale;
            float alpha = activation->config.activate_params[0];
            float beta = activation->config.activate_params[1];
            float lower = - beta / alpha;
            float upper = (1.f / alpha) + lower;
            hard_swish_thread_context_t context = {
                    .lower = lower * in_scale,
                    .upper = upper * in_scale,
                    .alpha = (alpha * out_scale) / (in_scale * in_scale),
                    .beta = (beta * out_scale) / in_scale,
                    .requantize = requantize,
                    .output = top->data.data
            };

            PARALLELIZE_1D_TILE_1D(hard_swish_thread_tile, context, size, group_size);
            return CNET_STATUS_SUCCESS;
        }
        case log_activate_type:{
            log_thread_context_t context = {
                    .out_scale = top->blob->scale,
                    .in_scale = bottom->blob->scale,
                    .value_base = activation->config.activate_params[0],
                    .scale = activation->config.activate_params[1],
                    .shift = activation->config.activate_params[2],
                    .output = bottom_top_blob->data,
            };

            if (context.value_base == -1.f){
                PARALLELIZE_1D_TILE_1D(log1_thread_tile, context, size, group_size);
                return CNET_STATUS_SUCCESS;
            }
            else{
                PARALLELIZE_1D_TILE_1D(log2_thread_tile, context, size, group_size);
                return CNET_STATUS_SUCCESS;
            }
        }
        case power_activate_type:{
            power_thread_context_t context = {
                    .out_scale = top->blob->scale,
                    .in_scale = bottom->blob->scale,
                    .power = activation->config.activate_params[0],
                    .scale = activation->config.activate_params[1],
                    .shift = activation->config.activate_params[2],
                    .output = bottom_top_blob->data,
            };

            PARALLELIZE_1D_TILE_1D(power_thread_tile, context, size, group_size);
            return CNET_STATUS_SUCCESS;
        }
        case relu_activate_type: {
            float out_scale = top->blob->scale / bottom->blob->scale;
            float slope = out_scale * activation->config.activate_params[0];
            relu_context_t context = {
                    .requantize = get_fixed_mul(out_scale),
                    .slope_fixed = get_fixed_mul(slope),
                    .output = bottom_top_blob->data,
            };

            PARALLELIZE_1D_TILE_1D(relu_thread_tile, context, size, group_size);
            return CNET_STATUS_SUCCESS;
        }
        case selu_activate_type:{
            selu_thread_context_t context = {
                    .out_scale = top->blob->scale,
                    .in_scale = bottom->blob->scale,
                    .alphaxlambda = activation->config.activate_params[0],
                    .lambda = activation->config.activate_params[1],
                    .output = bottom_top_blob->data,
            };

            PARALLELIZE_1D_TILE_1D(selu_thread_tile, context, size, group_size);
            return CNET_STATUS_SUCCESS;
        }
        case swish_activate_type:{

            swish_thread_context_t context = {
                .in_scale = top->blob->scale,
                .out_scale = top->blob->scale,
                .output = top->data.data
            };

            PARALLELIZE_1D_TILE_1D(swish_thread_tile, context, size, group_size);
            return CNET_STATUS_SUCCESS;
        }
        case threshold_activate_type:{
            float in_scale = bottom->blob->scale;
            threshold_context_t context = {
                    .threshold = float2int16(activation->config.activate_params[0] * in_scale),
                    .output = bottom_top_blob->data,
            };

            PARALLELIZE_1D_TILE_1D(threshold_thread_tile, context, size, group_size);
            return CNET_STATUS_SUCCESS;
        }
    }

    return CNET_STATUS_FAILED;
}

IMPL_OPERATION_CREATOR(activation) {
    operation_t *activation = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == activation) {
        printf("malloc for activation failed\n");
        return NULL;
    }

    operation_basic_info_setup(activation);
    activation->forward = activation_forward;
    activation->support_inplace = 1;
    return (operation_ptr) activation;
}
