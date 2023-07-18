/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "operation.h"
#include "operation_config.h"

typedef struct prelu_group_context_t {
    int group_size;
    float num_slope;
    float *slope_data;
    fixed_mul_t requantize;
    int16_t *ptr;
} prelu_group_context_t;

FUNCTION_IRAM static void prelu_int16_group_thread(prelu_group_context_t *context, size_t g) {
    int16_t *ptr = context->ptr + g * context->group_size;
    float slope = context->num_slope > 1 ? context->slope_data[g] : context->slope_data[0];
    fixed_mul_t slope_fixed = get_fixed_mul(slope);
    fixed_mul_t requantize = context->requantize;

    for (int j = 0; j < context->group_size; j++) {
        int32_t v = ptr[j];
        v = v < 0 ? MULTIPLY_FIDED(v, slope_fixed) : MULTIPLY_FIDED(v, requantize);
        ptr[j] = CLIP_INT16(v, INT16_MAX, INT16_MIN);
    }
}

FUNCTION_IRAM static int prelu_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    blob_container_t *top = &top_tensors->data[0];
    prelu_t *prelu = (prelu_t*)operation->base;

    float *slope_data = (float *) prelu->slope.data.data;
    int group = top->blob->shape[3];
    int group_size = 1;

    if(top->blob->shape[1] == 1 && top->blob->shape[2] > 1 ){
        group =  top->blob->shape[2];
        group_size = top->blob->shape[3];
    } else if(top->blob->shape[1] > 1){
        group =  top->blob->shape[1];
        group_size = top->blob->shape[2]*top->blob->shape[3];
    }

    prelu_group_context_t context = {
            group_size,
            prelu->config.num_slope,
            slope_data,
            prelu->config.requantize,
            top->data.data
    };

    PARALLELIZE_1D(prelu_int16_group_thread, context, group);
    return CNET_STATUS_SUCCESS;
}

IMPL_OPERATION_CREATOR(prelu) {
    operation_t *prelu = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == prelu) {
        printf("malloc for padding failed\n");
        return NULL;
    }

    operation_basic_info_setup(prelu);
    prelu->forward = prelu_forward;
    prelu->support_inplace = 1;
    return (operation_ptr) prelu;
}
