/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "tensor_operation.h"


#undef Min
#undef Max
#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))

static void copy_cut_border_image_int8(const tensor_t *src, tensor_t *dst, int top, int left) {
    int w = dst->d0;
    int h = dst->d1;

    const int8_t *ptr = ((int8_t *) src->data) + src->d0 * top + left;
    int8_t *out_ptr = dst->data;

    for (int y = 0; y < h; y++) {
        if (w < 12) {
            for (int x = 0; x < w; x++) {
                out_ptr[x] = ptr[x];
            }
        } else {
            memcpy(out_ptr, ptr, w * sizeof(int8_t));
        }
        out_ptr += w;
        ptr += src->d0;
    }
}

static void copy_cut_border_image_int16(const tensor_t *src, tensor_t *dst, int top, int left) {
    int w = dst->d0;
    int h = dst->d1;

    const int16_t *ptr = ((int16_t *) src->data) + src->d0 * top + left;
    int16_t *out_ptr = dst->data;

    for (int y = 0; y < h; y++) {
        if (w < 12) {
            for (int x = 0; x < w; x++) {
                out_ptr[x] = ptr[x];
            }
        } else {
            memcpy(out_ptr, ptr, w * sizeof(int16_t));
        }
        out_ptr += w;
        ptr += src->d0;
    }
}


static void copy_cut_border_image_float(const tensor_t *src, tensor_t *dst, int top, int left) {
    int w = dst->d0;
    int h = dst->d1;

    const float *ptr = ((float *) src->data) + src->d0 * top + left;
    float *out_ptr = dst->data;

    for (int y = 0; y < h; y++) {
        if (w < 12) {
            for (int x = 0; x < w; x++) {
                out_ptr[x] = ptr[x];
            }
        } else {
            memcpy(out_ptr, ptr, w * sizeof(float));
        }
        out_ptr += w;
        ptr += src->d0;
    }
}


tensor_t tensor_cut_border(
        tensor_t *src,
        int top, int bottom,
        int left, int right,
        option_t *opt){

    int w = src->d0 - left - right;
    int h = src->d1 - top - bottom;

    tensor_t dst = tensor_create_default();
    tensor_create_3d(&dst, w, h, src->d2, src->elem_size, opt ? opt->tensor : NULL);
    tensor_set_data_type(&dst, src->data_type);

    if(tensor_empty(&dst)){
        return dst;
    }

    for (int q=0; q<src->d2; q++)
    {
        const tensor_t m = tensor_d2(src, q);
        tensor_t border_m = tensor_d2(&dst, q);

        if(src->elem_size == 1){
            copy_cut_border_image_int8(&m, &border_m, top, left);
        } else if(src->elem_size == 2){
            copy_cut_border_image_int16(&m, &border_m, top, left);
        }else if(src->elem_size == 4){
            copy_cut_border_image_float(&m, &border_m, top, left);
        }
    }

    return dst;
}