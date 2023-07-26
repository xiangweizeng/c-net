
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"

FUNCTION_IRAM static int memory_data_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    memory_data_t *memory_data = (memory_data_t *) operation->base;
    tensor_t *top_tensor = &top_tensors->data[0].data;

    size_t total_size = tensor_total(top_tensor) * top_tensor->elem_size;
    memcpy(top_tensor->data, memory_data->data.data, total_size);
    return CNET_STATUS_SUCCESS;
}

IMPL_OPERATION_CREATOR(memory_data) {
    operation_t *memory_data = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == memory_data) {
        printf("malloc for memory_data failed\n");
        return NULL;
    }

    operation_basic_info_setup(memory_data);
    memory_data->forward = memory_data_forward;
    return (operation_ptr) memory_data;
}