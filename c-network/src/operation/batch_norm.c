
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"

FUNCTION_IRAM static int batch_norm_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    batch_norm_t *batch_norm = (batch_norm_t *) operation->base;
    blob_container_t *top = &top_tensors->data[0];

    int32_t *a_data = batch_norm->offset.data;
    float *b_data = batch_norm->scale.data;

    int group =  top->blob->shape[1] * top->blob->shape[2];
    int group_size = top->blob->shape[3];
    int16_t *ptr = top->data.data;

    for(int s = 0; s < group; s ++){
        for(int j = 0; j < group_size; j++){
            int32_t a = a_data[j];
            float b = b_data[j];
            float v = b * ptr[j] + a;
            ptr[j] = float2int16(v);
        }
        ptr += group_size;
    }

    return CNET_STATUS_SUCCESS;
}

IMPL_OPERATION_CREATOR(batch_norm) {
    operation_t *batch_norm = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == batch_norm) {
        printf("malloc for batch_norm failed\n");
        return NULL;
    }

    operation_basic_info_setup(batch_norm);
    batch_norm->forward = batch_norm_forward;
    batch_norm->support_inplace = 1;
    return (operation_ptr) batch_norm;
}
