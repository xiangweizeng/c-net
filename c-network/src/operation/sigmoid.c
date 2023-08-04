
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"


FUNCTION_IRAM static int sigmoid_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    tensor_t *bottom_tensor = &bottom_tensors->data[0].data;
    tensor_t *top_tensor = &top_tensors->data[0].data;
    const blob_info_t *bottom_blob_info = bottom_tensors->data[0].blob;

    quantize_tensor(
            bottom_tensor,
            top_tensor,
            bottom_blob_info->scale,
            opt);

    int size = tensor_total(top_tensor);
    float* ptr = (float*)top_tensor->data;

    for (int i = 0; i < size; i++) {
        ptr[i] = 1.f / (1.f + exp(-ptr[i]));
    }

    return CNET_STATUS_SUCCESS;
}

IMPL_OPERATION_CREATOR(sigmoid) {
    operation_t *sigmoid = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == sigmoid) {
        printf("malloc for sigmoid failed\n");
        return NULL;
    }

    operation_basic_info_setup(sigmoid);
    sigmoid->forward = sigmoid_forward;
    return (operation_ptr) sigmoid;
}