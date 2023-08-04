/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"

FUNCTION_IRAM static int permute_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    permute_t *permute = (permute_t *) operation->base;
    tensor_t *bottom_tensor = &bottom_tensors->data[0].data;
    tensor_t *top_tensor = &top_tensors->data[0].data;

    permute_config_t config = permute->config;

    const int * in_shape = bottom_tensors->data[0].blob->shape;
    const int* out_shape = top_tensors->data[0].blob->shape;

    int8_t* input = (int8_t*)bottom_tensor->data;
    int8_t* output = (int8_t*)top_tensor->data;

    blob_shape_t i, o;
    for (o[3] = 0; o[3] < out_shape[3]; o[3]++) {
        i[config.axis[3]] = o[3];
        for (o[2] = 0; o[2] < out_shape[2]; o[2]++) {
            i[config.axis[2]] = o[2];
            for (o[1] = 0; o[1] < out_shape[1]; o[1]++) {
                i[config.axis[1]] = o[1];
                for (o[0] = 0; o[0] < out_shape[0]; o[0]++) {
                    i[config.axis[0]] = o[0];
                    output[BLOB_SHAPE_OFFSET(out_shape, o)] = input[BLOB_SHAPE_OFFSET(in_shape, i)];
                }
            }
        }
    }

    return CNET_STATUS_SUCCESS;
}

IMPL_OPERATION_CREATOR(permute) {
    operation_t *permute = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == permute) {
        printf("malloc for permute failed\n");
        return NULL;
    }

    operation_basic_info_setup(permute);
    permute->forward = permute_forward;
    return (operation_ptr) permute;
}
