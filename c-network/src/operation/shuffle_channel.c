
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"

FUNCTION_IRAM static int shuffle_channel_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    shuffle_channel_t *shuffle_channel = (shuffle_channel_t *) operation->base;
    tensor_t *bottom_tensor = &bottom_tensors->data[0].data;
    tensor_t *top_tensor = &top_tensors->data[0].data;

    int w = bottom_tensor->d0;
    int h = bottom_tensor->d1;
    int c = bottom_tensor->d2;
    int group = shuffle_channel->config.group;

    size_t elem_size = bottom_tensor->elem_size;
    int chs_per_group = c / group;
    if (c != chs_per_group * group)
    {
        // reject invalid group
        return CNET_STATUS_FAILED;
    }

    const size_t feature_sz = w * h * elem_size;
    for (int i = 0; i != group; i++)
    {
        for (int j = 0; j != chs_per_group; j++)
        {
            int src_q = chs_per_group * i + j;
            int dst_q = group * j + i;
            memcpy(tensor_d2(top_tensor,dst_q).data, tensor_d2(bottom_tensor, src_q).data, feature_sz);
        }
    }

    return CNET_STATUS_SUCCESS;
}

IMPL_OPERATION_CREATOR(shuffle_channel) {
    operation_t *shuffle_channel = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == shuffle_channel) {
        printf("malloc for shuffle_channel failed\n");
        return NULL;
    }

    operation_basic_info_setup(shuffle_channel);
    shuffle_channel->forward = shuffle_channel_forward;
    return (operation_ptr) shuffle_channel;
}