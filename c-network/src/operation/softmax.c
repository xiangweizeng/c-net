
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include <float.h>

#include "operation.h"
#include "operation_config.h"

#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))

FUNCTION_IRAM static int softmax_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    softmax_t *softmax = (softmax_t *) operation->base;
    tensor_t *bottom_tensor = &bottom_tensors->data[0].data;
    tensor_t *top_tensor = &top_tensors->data[0].data;
    softmax_config_t config = softmax->config;
    const int * out_shape = top_tensors->data[0].blob->shape;

    quantize_tensor(
            bottom_tensor,
            top_tensor,
            bottom_tensors->data[0].blob->scale,
            opt);

    // value = exp( value - global max value )
    // sum all value
    // value = value / sum

    // temp shape
    blob_shape_t temp_shape;
    temp_shape[0] = 1;
    for (int i = 1; i < 4; i++) {
        if (config.axis == i) {
            temp_shape[i] = 1;
        }
        else {
            temp_shape[i] = out_shape[i];
        }
    }

    tensor_t max = tensor_create_default();
    tensor_create_3d(&max, temp_shape[1], temp_shape[2], temp_shape[3], 4u, NULL);
    tensor_t sum = tensor_create_default();
    tensor_create_3d(&sum, temp_shape[1], temp_shape[2], temp_shape[3], 4u, NULL);

    float* ptr = (float*)top_tensor->data;
    float* max_data = (float*)max.data;
    float* sum_data = (float*)sum.data;
    register int axis = config.axis;
    register int touch_step = out_shape[axis];

    blob_shape_t i, o;
    for (i[0] = 0; i[0] < out_shape[0]; i[0]++) {
        /// o[0] = 0
        o[0] = 0;

        /// max value
        for (o[1] = 0; o[1] < temp_shape[1]; o[1]++) {
            i[1] = o[1];
            for (o[2] = 0; o[2] < temp_shape[2]; o[2]++) {
                i[2] = o[2];
                for (o[3] = 0; o[3] < temp_shape[3]; o[3]++) {
                    i[3] = o[3];
                    register float max_value = -FLT_MAX;
                    for (int t = 0; t < touch_step; t++) {
                        i[axis] = t;
                        max_value = Max(max_value, ptr[BLOB_SHAPE_OFFSET(out_shape, i)]);
                    }
                    max_data[BLOB_SHAPE_OFFSET(temp_shape, o)] = max_value;
                }
            }
        }

        /// sum value
        for (o[1] = 0; o[1] < temp_shape[1]; o[1]++) {
            i[1] = o[1];
            for (o[2] = 0; o[2] < temp_shape[2]; o[2]++) {
                i[2] = o[2];
                for (o[3] = 0; o[3] < temp_shape[3]; o[3]++) {
                    i[3] = o[3];
                    register float sum_value = 0;
                    register float max_value = max_data[BLOB_SHAPE_OFFSET(temp_shape, o)];
                    for (int t = 0; t < touch_step; t++) {
                        i[axis] = t;
                        float temp_value = exp(ptr[BLOB_SHAPE_OFFSET(out_shape, i)] - max_value);
                        sum_value += temp_value;
                        ptr[BLOB_SHAPE_OFFSET(out_shape, i)] = temp_value;

                    }

                    sum_data[BLOB_SHAPE_OFFSET(temp_shape, o)] = sum_value;
                }
            }
        }

        /// value / sum
        for (o[1] = 0; o[1] < temp_shape[1]; o[1]++) {
            i[1] = o[1];
            for (o[2] = 0; o[2] < temp_shape[2]; o[2]++) {
                i[2] = o[2];
                for (o[3] = 0; o[3] < temp_shape[3]; o[3]++) {
                    i[3] = o[3];
                    register float sum_value = sum_data[BLOB_SHAPE_OFFSET(temp_shape, o)];
                    for (int t = 0; t < touch_step; t++) {
                        i[axis] = t;
                        ptr[BLOB_SHAPE_OFFSET(out_shape, i)] /= sum_value;
                    }
                }
            }
        }
    }

    tensor_release(&max);
    tensor_release(&sum);

    return CNET_STATUS_SUCCESS;
}

IMPL_OPERATION_CREATOR(softmax) {
    operation_t *softmax = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == softmax) {
        printf("malloc for softmax failed\n");
        return NULL;
    }

    operation_basic_info_setup(softmax);
    softmax->forward = softmax_forward;
    return (operation_ptr) softmax;
}