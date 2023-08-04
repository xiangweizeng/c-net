/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "operation.h"
#include "operation_config.h"

#undef Min
#undef Max
#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))

static void copy_cut_border_image_int8(
        const tensor_t *src,
        tensor_t *dst,
        int top,
        int left,
        int front) {

    int w = dst->d1;
    int h = dst->d2;
    int c = dst->d0;
    int ic = src->d0;

    int8_t *out_ptr = dst->data;
    for (int y = 0; y < h; y++) {
        const int8_t *ptr = ((int8_t *) src->data) + ((top + y) * src->d1 + left) * src->d0;
        for (int x = 0; x < w; x++) {
            for(int z = 0; z < c; z++){
                out_ptr[z] = ptr[z + front];
            }
            out_ptr += c;
            out_ptr += ic;
        }
    }
}

static int crop_forward_impl(
        operation_t *operation,
        tensor_t *bottom_tensor,
        tensor_t *top_tensor,
        option_t *opt) {

    crop_t *crop = (crop_t *) operation->base;
    crop_config_t config = crop->config;

    if (config.dims == 1)
    {
        copy_cut_border_image_int8(bottom_tensor, top_tensor, 0, config.w_offset, 0);
        return CNET_STATUS_SUCCESS;
    }

    if (config.dims == 2)
    {
        copy_cut_border_image_int8(bottom_tensor, top_tensor, config.h_offset, config.w_offset, 0);
        return CNET_STATUS_SUCCESS;
    }

    if (config.dims == 3)
    {
        copy_cut_border_image_int8(bottom_tensor, top_tensor, config.h_offset, config.w_offset, config.c_offset);
        return CNET_STATUS_SUCCESS;
    }

    return CNET_STATUS_SUCCESS;
}

FUNCTION_IRAM static int crop_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    tensor_t *bottom_tensor = &bottom_tensors->data[0].data;
    tensor_t *top_tensor = &top_tensors->data[0].data;

    return crop_forward_impl(operation, bottom_tensor, top_tensor, opt);
}

IMPL_OPERATION_CREATOR(crop) {
    operation_t *crop = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == crop) {
        printf("malloc for padding failed\n");
        return NULL;
    }

    operation_basic_info_setup(crop);
    crop->forward = crop_forward;
    return (operation_ptr) crop;
}
