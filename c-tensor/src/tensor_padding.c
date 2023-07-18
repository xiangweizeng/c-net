/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "tensor_operation.h"


static void copy_make_border_image_int8(
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


static void copy_make_border_image_int16(
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

static void copy_make_border_image_float(
        tensor_t *src, tensor_t *dst,
        int top, int left, int type, int16_t v) {

    int w = dst->d0;
    int h = dst->d1;

    const float *ptr = (float *) src->data;
    float *out_ptr = (float *) dst->data;

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
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(float));
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
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(float));
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
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(float));
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
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(float));
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
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(float));
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
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(float));
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
                memcpy(out_ptr + left, ptr, src->d0 * sizeof(float));
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
    int top;
    int left;
    border_type_t type;
    float v;
    size_t elem_size;
}padding_channels_context_t;

FUNCTION_IRAM void padding_channels_thread(padding_channels_context_t *context, int q) {
    tensor_t m = tensor_d2(context->bottom_tensor, q);
    tensor_t border_m = tensor_d2(context->top_tensor, q);

    if (context->elem_size == 1){
        copy_make_border_image_int8(
                &m, &border_m,
                context->top,
                context->left,
                context->top,
                context->v);
    }
    else if (context->elem_size == 2){
        copy_make_border_image_int16(
                &m, &border_m,
                context->top,
                context->left,
                context->top,
                context->v);
    } else if (context->elem_size == 4){
        copy_make_border_image_float(
                &m, &border_m,
                context->top,
                context->left,
                context->top,
                context->v);
    }
}

tensor_t tensor_padding(
        tensor_t *src,
        int top,
        int bottom,
        int left,
        int right,
        border_type_t type,
        float v,
        option_t *opt){

    int channels = src->d2;
    size_t elem_size = src->elem_size;

    int w = src->d0 + left + right;
    int h = src->d1 + top + bottom;

    tensor_t dst = tensor_create_default();
    tensor_create_3d(&dst, w, h, src->d2, src->elem_size, opt->tensor);
    tensor_set_data_type(&dst, src->data_type);

    padding_channels_context_t context = {src, &dst, top, left, type, v, elem_size};
    PARALLELIZE_1D(padding_channels_thread, context, channels);

}
