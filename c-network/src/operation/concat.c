
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"

FUNCTION_IRAM static int concat_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    concat_t *concat = (concat_t *) operation->base;
    concat_config_t config = concat->config;
    tensor_t *top_tensor = &top_tensors->data[0].data;

    int16_t* output = (int16_t*)top_tensor->data;
    const int* out_shape = top_tensors->data[0].blob->shape;

    register int b, concat_start = 0, axis = config.axis;
    for (b = 0; b < config.concat_count; b++) {
        tensor_t* input_matrix = &bottom_tensors->data[b].data;
        const int * in_shape = bottom_tensors->data[b].blob->shape;

        if (tensor_total(input_matrix) == 0) {
            return CNET_STATUS_FAILED;
        }

        int16_t* input = (int16_t*)input_matrix->data;
        fixed_mul_t fixed = get_fixed_mul(config.output_requantize[b]);

        blob_shape_t i; i[0] = 0;
        for (i[1] = 0; i[1] < in_shape[1]; i[1]++) {
            for (i[2] = 0; i[2] < in_shape[2]; i[2]++) {
                for (i[3] = 0; i[3] < in_shape[3]; i[3]++) {
                    blob_shape_t o = {0, i[1], i[2], i[3]};
                    o[axis] += concat_start;
                    output[BLOB_SHAPE_OFFSET(out_shape, o)] =
                            MULTIPLY_FIDED(input[BLOB_SHAPE_OFFSET(in_shape, i)], fixed);
                }
            }
        }

        concat_start += in_shape[axis];
    }

    return CNET_STATUS_SUCCESS;
}

IMPL_OPERATION_CREATOR(concat) {
    operation_t *concat = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == concat) {
        printf("malloc for concat failed\n");
        return NULL;
    }

    operation_basic_info_setup(concat);
    concat->forward = concat_forward;
    return (operation_ptr) concat;
}
