
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "quantize16.h"

typedef struct quantize_input_context {
    void* input;
    void* output;
    size_t group_size;
    tensor_data_type_t src_type;
    tensor_data_type_t dest_type;
    float scale;
} quantize_input_context_t;

FUNCTION_IRAM static void quantize_input_thread(quantize_input_context_t* context, size_t index, size_t tile)
{
    if (context->src_type == TENSOR_DATA_FLOAT) {
        float* ptr = (float*)context->input + index;
        int16_t* outptr = (int16_t*)context->output + index;

        size_t i;
        for (i = 0; i < tile; i++) {
            outptr[i] = float2int16(ptr[i] * context->scale);
        }
    }
    else if (context->dest_type == TENSOR_DATA_FLOAT) {
        float* outptr = (float*)context->output + index;
        int16_t* ptr = (int16_t*)context->input + index;

        size_t i;
        for (i = 0; i < tile; i++) {
            outptr[i] = (ptr[i] / context->scale);
        }
    }
}


void quantize16_tensor_parallize(tensor_t* bottom_tensor, tensor_t* top_tensor, float scale, option_t* opt)
{
    size_t  size = tensor_total(bottom_tensor);
    int thread_number = opt->thread_number > 0 ? opt->thread_number : 1;

    size_t group_size = size / thread_number;
    group_size = group_size > 0 ? group_size : size;

    quantize_input_context_t context = {
        bottom_tensor->data, 
        top_tensor->data, 
        group_size, 
        bottom_tensor->data_type, 
        top_tensor->data_type, 
        scale 
    };

    PARALLELIZE_1D_TILE_1D(quantize_input_thread, context, size, group_size);
}