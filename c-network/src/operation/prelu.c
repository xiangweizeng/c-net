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
    int group =  top->blob->shape[1] * top->blob->shape[2];
    int group_size = top->blob->shape[3];

    if(prelu->config.num_slope == 1){
        size_t total = group * group_size;
        int16_t *ptr = top->data.data;
        float slope = slope_data[0];
        fixed_mul_t slope_fixed = get_fixed_mul(slope);
        fixed_mul_t requantize = prelu->config.requantize;

        for (int j = 0; j < total; j++) {
            int32_t v = ptr[j];
            v = v < 0 ? MULTIPLY_FIDED(v, slope_fixed) : MULTIPLY_FIDED(v, requantize);
            ptr[j] = CLIP_INT16(v, INT16_MAX, INT16_MIN);
        }
    }else{
        int16_t *ptr = top->data.data;
        fixed_mul_t requantize = prelu->config.requantize;
        for(int s = 0; s < group; s ++){
            for(int j = 0; j < group_size; j++){
                float slope = slope_data[j];
                fixed_mul_t slope_fixed = get_fixed_mul(slope);

                int32_t v = ptr[j];
                v = v < 0 ? MULTIPLY_FIDED(v, slope_fixed) : MULTIPLY_FIDED(v, requantize);
                ptr[j] = CLIP_INT16(v, INT16_MAX, INT16_MIN);
            }
            ptr += group_size;
        }
    }

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
