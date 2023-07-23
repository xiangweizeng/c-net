
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"


FUNCTION_IRAM static int batch_norm_forward_one_inplace(batch_norm_t *batch_norm, tensor_t *tensor, option_t *opt) {


    int16_t *a_data = batch_norm->offset.data.data;
    int16_t *b_data = batch_norm->scale.data.data;
    fixed_mul_t requantize = batch_norm->config.requantize;

    int group =  top->blob->shape[1] * top->blob->shape[2];
    int group_size = top->blob->shape[3];

    int dims = tensor->dims;
    if (dims == 1)
    {
        int w = tensor->d0;
        int16_t* ptr = tensor->data;
        for (int i=0; i<w; i++)
        {
            int32_t v = b_data[i] * ptr[i];
            ptr[i] =  REQUANTIZE_BIAS(v, requantize, a_data[i]);
        }
    }

    if (dims == 2)
    {
        int w = tensor->d0;
        int h = tensor->d1;

        for (int i=0; i<h; i++)
        {
            int16_t* ptr = (int16_t *)tensor + i * tensor->d0;
            int16_t a = a_data[i];
            int16_t b = b_data[i];

            for (int j=0; j<w; j++)
            {
                int32_t v = b * ptr[i];
                ptr[i] = REQUANTIZE_BIAS(v, requantize, a);
            }
        }
    }

    if (dims == 3)
    {
        int w = tensor->d0;
        int h = tensor->d1;
        int size = w * h;

        for (int q=0; q < tensor->d2; q++)
        {
            int16_t* ptr = tensor_d2(tensor, q).data;
            int16_t a = a_data[q];
            int16_t b = b_data[q];

            for (int i=0; i<size; i++)
            {
                int32_t v = b * ptr[i];
                ptr[i] = REQUANTIZE_BIAS(v, requantize, a);
            }
        }
    }

    return CNET_STATUS_SUCCESS;
}

FUNCTION_IRAM static int batch_norm_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    batch_norm_t *batch_norm = (batch_norm_t *) operation->base;
    blob_container_t *top = &top_tensors->data[0];

    int16_t *a_data = batch_norm->offset.data.data;
    int16_t *b_data = batch_norm->scale.data.data;
    fixed_mul_t requantize = batch_norm->config.requantize;

    int group =  top->blob->shape[1] * top->blob->shape[2];
    int group_size = top->blob->shape[3];
    int16_t *ptr = top->data.data;

    for(int s = 0; s < group; s ++){
        for(int j = 0; j < group_size; j++){
            int16_t a = a_data[j];
            int16_t b = b_data[j];
            int32_t v = b * ptr[j];
            ptr[j] = REQUANTIZE_BIAS(v, requantize, a);
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
