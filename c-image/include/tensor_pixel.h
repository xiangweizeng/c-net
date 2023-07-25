/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_TENSOR_PIXEL_H
#define CNET_TENSOR_PIXEL_H

#include "tensor.h"

typedef enum pixel_type_t
{
    PIXEL_CONVERT_SHIFT = 16,
    PIXEL_FORMAT_MASK = 0x0000ffff,
    PIXEL_CONVERT_MASK = 0xffff0000,

    PIXEL_RGB = 1,
    PIXEL_BGR = (1 << 1),
    PIXEL_GRAY = (1 << 2),
    PIXEL_RGBA = (1 << 3),

    PIXEL_RGB2BGR = PIXEL_RGB | (PIXEL_BGR << PIXEL_CONVERT_SHIFT),
    PIXEL_RGB2GRAY = PIXEL_RGB | (PIXEL_GRAY << PIXEL_CONVERT_SHIFT),

    PIXEL_BGR2RGB = PIXEL_BGR | (PIXEL_RGB << PIXEL_CONVERT_SHIFT),
    PIXEL_BGR2GRAY = PIXEL_BGR | (PIXEL_GRAY << PIXEL_CONVERT_SHIFT),

    PIXEL_GRAY2RGB = PIXEL_GRAY | (PIXEL_RGB << PIXEL_CONVERT_SHIFT),
    PIXEL_GRAY2BGR = PIXEL_GRAY | (PIXEL_BGR << PIXEL_CONVERT_SHIFT),

    PIXEL_RGBA2RGB = PIXEL_RGBA | (PIXEL_RGB << PIXEL_CONVERT_SHIFT),
    PIXEL_RGBA2BGR = PIXEL_RGBA | (PIXEL_BGR << PIXEL_CONVERT_SHIFT),
    PIXEL_RGBA2GRAY = PIXEL_RGBA | (PIXEL_GRAY << PIXEL_CONVERT_SHIFT),
} pixel_type_t;

/**
 * mean norm operator
 */
FUNCTION_IRAM void tensor_substract_mean_normalize(tensor_t *tensor, const float* mean_vals, const float* norm_vals);

/**
 * create tensor form pixels data, return tensor is nchw
 */
FUNCTION_IRAM tensor_t tensor_from_pixels(const unsigned char* pixels, int type, int w, int h, allocator_t *allocator);
FUNCTION_IRAM tensor_t tensor_from_pixels_resize(const unsigned char* pixels, int type, int w, int h, int target_width, int target_height, allocator_t *allocator);
//FUNCTION_IRAM tensor_t tensor_from_float16(const unsigned short *data, int size, allocator_t *allocator);// create tensor form float16 buffer

/**
 * tensor data to pixels data, tensor is nchw layout
 */
FUNCTION_IRAM void tensor_to_pixels(tensor_t *tensor, unsigned char* pixels, int type);
FUNCTION_IRAM void tensor_to_pixels_resize(tensor_t *tensor, unsigned char* pixels, int type, int target_width, int target_height, allocator_t *allocator);

/**
 * draw box on rgb tensor, color, rgb, box: x1,y1, x2,y2
 */
FUNCTION_IRAM void draw_rgb_image_box_width(tensor_t a, int x1, int y1, int x2, int y2, int w, float r, float g, float b);
FUNCTION_IRAM void draw_rgb_image_point_width(tensor_t a, int x1, int y1, int w, float r, float g, float b);

#endif //CNET_TENSOR_PIXEL_H
