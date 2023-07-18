
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"

FUNCTION_IRAM static void copy_make_border_image_int8(
        tensor_t *src, tensor_t *dst,
        int top, int left, int type, int8_t v) {

    int w = dst->d0;
    int h = dst->d1;

    const int8_t *ptr = (int8_t *) src->data;
    int8_t *out_ptr = (int8_t *) dst->data;

    if (type == 0) {
        int y = 0;

        // fill top
        for (; y < top; y++) {
            int x = 0;
            for (; x < w; x++) {
                out_ptr[x] = v;
            }
            out_ptr += w;
        }

        // fill center
        for (; y < (top + src->d1); y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = v;
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int8_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = v;
            }
            ptr += src->d0;
            out_ptr += w;
        }

        // fill bottom
        for (; y < h; y++) {
            int x = 0;
            for (; x < w; x++) {
                out_ptr[x] = v;
            }
            out_ptr += w;
        }
    } else if (type == 1) {
        int y = 0;

        // fill top
        for (; y < top; y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[0];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int8_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - 1];
            }
            out_ptr += w;
        }

        // fill center
        for (; y < (top + src->d1); y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[0];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int8_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - 1];
            }
            ptr += src->d0;
            out_ptr += w;
        }

        // fill bottom
        ptr -= src->d0;
        for (; y < h; y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[0];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int8_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - 1];
            }
            out_ptr += w;
        }
    } else if (type == 2) {
        int y = 0;

        // fill top
        ptr += top * src->d0;
        for (; y < top; y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[left - x];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int8_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - (x - left - src->d0) - 2];
            }
            out_ptr += w;
            ptr -= src->d0;
        }

        // fill center
        for (; y < (top + src->d1); y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[left - x];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int8_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - (x - left - src->d0) - 2];
            }
            ptr += src->d0;
            out_ptr += w;
        }

        // fill bottom
        ptr -= 2 * src->d0;
        for (; y < h; y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[left - x];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int8_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - (x - left - src->d0) - 2];
            }
            out_ptr += w;
            ptr -= src->d0;
        }
    }
}


FUNCTION_IRAM static void copy_make_border_image_int16(
        tensor_t *src, tensor_t *dst,
        int top, int left, int type, int16_t v) {

    int w = dst->d0;
    int h = dst->d1;

    const int16_t *ptr = (int16_t *) src->data;
    int16_t *out_ptr = (int16_t *) dst->data;

    if (type == 0) {
        int y = 0;

        // fill top
        for (; y < top; y++) {
            int x = 0;
            for (; x < w; x++) {
                out_ptr[x] = v;
            }
            out_ptr += w;
        }

        // fill center
        for (; y < (top + src->d1); y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = v;
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int16_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = v;
            }
            ptr += src->d0;
            out_ptr += w;
        }

        // fill bottom
        for (; y < h; y++) {
            int x = 0;
            for (; x < w; x++) {
                out_ptr[x] = v;
            }
            out_ptr += w;
        }
    } else if (type == 1) {
        int y = 0;

        // fill top
        for (; y < top; y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[0];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int16_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - 1];
            }
            out_ptr += w;
        }

        // fill center
        for (; y < (top + src->d1); y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[0];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int16_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - 1];
            }
            ptr += src->d0;
            out_ptr += w;
        }

        // fill bottom
        ptr -= src->d0;
        for (; y < h; y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[0];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int16_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - 1];
            }
            out_ptr += w;
        }
    } else if (type == 2) {
        int y = 0;

        // fill top
        ptr += top * src->d0;
        for (; y < top; y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[left - x];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int16_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - (x - left - src->d0) - 2];
            }
            out_ptr += w;
            ptr -= src->d0;
        }

        // fill center
        for (; y < (top + src->d1); y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[left - x];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int16_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - (x - left - src->d0) - 2];
            }
            ptr += src->d0;
            out_ptr += w;
        }

        // fill bottom
        ptr -= 2 * src->d0;
        for (; y < h; y++) {
            int x = 0;
            for (; x < left; x++) {
                out_ptr[x] = ptr[left - x];
            }
            if (src->d0 < 12) {
                for (; x < (left + src->d0); x++) {
                    out_ptr[x] = ptr[x - left];
                }
            } else {
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(int16_t));
                x += src->d0;
            }
            for (; x < w; x++) {
                out_ptr[x] = ptr[src->d0 - (x - left - src->d0) - 2];
            }
            out_ptr += w;
            ptr -= src->d0;
        }
    }
}


typedef struct padding_channels_context_t{
    tensor_t *bottom_tensor;
    tensor_t *top_tensor;
    padding_t *padding;
    size_t elem_size;
}padding_channels_context_t;

FUNCTION_IRAM void padding_channels_thread(padding_channels_context_t *context, int q) {
    tensor_t m = tensor_d2(context->bottom_tensor, q);
    tensor_t border_m = tensor_d2(context->top_tensor, q);

    padding_t *padding = context->padding;
    if (context->elem_size == 1){
        copy_make_border_image_int8(
                &m, &border_m,
                padding->config.pad_top,
                padding->config.pad_left,
                padding->config.pad_mode,
                padding->config.pad_value);
    }
    else if (context->elem_size == 2){
        copy_make_border_image_int16(
                &m, &border_m,
                padding->config.pad_top,
                padding->config.pad_left,
                padding->config.pad_mode,
                padding->config.pad_value);
    }
}


FUNCTION_IRAM static int padding_forward_impl(
        padding_t *padding,
        tensor_t *bottom_tensor,
        tensor_t *top_tensor,
        option_t *opt) {

    int channels = bottom_tensor->d2;
    int dims = bottom_tensor->dims;
    size_t elem_size = bottom_tensor->elem_size;

    if (dims == 1) {

        if (elem_size == 1){
            copy_make_border_image_int8(
                    bottom_tensor,
                    top_tensor,
                    0,
                    padding->config.pad_left,
                    padding->config.pad_mode,
                    padding->config.pad_value);
        }

        else if (elem_size == 2){
            copy_make_border_image_int16(
                    bottom_tensor,
                    top_tensor,
                    0,
                    padding->config.pad_left,
                    padding->config.pad_mode,
                    padding->config.pad_value);
        }

        return CNET_STATUS_SUCCESS;
    }
    
    if (dims == 2) {

        if (elem_size == 1)
            copy_make_border_image_int8(
                    bottom_tensor,
                    top_tensor,
                    padding->config.pad_top,
                    padding->config.pad_left,
                    padding->config.pad_mode,
                    padding->config.pad_value
                    );
        else if (elem_size == 2){
            copy_make_border_image_int16(
                    bottom_tensor,
                    top_tensor,
                    padding->config.pad_top,
                    padding->config.pad_left,
                    padding->config.pad_mode,
                    padding->config.pad_value
            );
        }

        return CNET_STATUS_SUCCESS;
    }

    if (dims == 3) {

        padding_channels_context_t context = {bottom_tensor, top_tensor, padding, elem_size};
        PARALLELIZE_1D(padding_channels_thread, context, channels);

        return CNET_STATUS_SUCCESS;
    }

    return CNET_STATUS_SUCCESS;
}

FUNCTION_IRAM static int padding_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    padding_t *padding = (padding_t *) operation->base;
    tensor_t *bottom_tensor = &bottom_tensors->data[0].data;
    tensor_t *top_tensor = &top_tensors->data[0].data;

    return padding_forward_impl(padding, bottom_tensor, top_tensor, opt);
}

IMPL_OPERATION_CREATOR(padding) {
    operation_t *padding = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == padding) {
        printf("malloc for padding failed\n");
        return NULL;
    }

    operation_basic_info_setup(padding);
    padding->forward = padding_forward;
    return (operation_ptr) padding;
}
