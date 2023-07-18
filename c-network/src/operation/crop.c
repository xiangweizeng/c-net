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

static void copy_cut_border_image_int16(const tensor_t *src, tensor_t *dst, int top, int left, fixed_mul_t req) {
    int w = dst->d0;
    int h = dst->d1;

    const int16_t *ptr = ((int16_t *) src->data) + src->d0 * top + left;
    int16_t *out_ptr = dst->data;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int v = MULTIPLY_FIDED(ptr[x], req);
            v = CLIP_INT16(v, INT16_MAX, INT16_MIN);
            out_ptr[x] = v;
        }

        out_ptr += w;
        ptr += src->d0;
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
        copy_cut_border_image_int16(bottom_tensor, top_tensor, 0, config.w_offset, config.requantize);
        return CNET_STATUS_SUCCESS;
    }

    if (config.dims == 2)
    {
        copy_cut_border_image_int16(bottom_tensor, top_tensor, config.h_offset, config.w_offset, config.requantize);
        return CNET_STATUS_SUCCESS;
    }

    if (config.dims == 3)
    {
        tensor_t bottom_blob_sliced = tensor_d2_range(bottom_tensor, config.c_offset, config.out_c);
        for (int q=0; q<config.out_c; q++)
        {
            const tensor_t m = tensor_d2(&bottom_blob_sliced, q);
            tensor_t border_m = tensor_d2(top_tensor, q);
            copy_cut_border_image_int16(&m, &border_m, config.h_offset, config.w_offset, config.requantize);
        }

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
