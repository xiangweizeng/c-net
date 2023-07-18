/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_PLATE_INFO_H
#define CNET_PLATE_INFO_H

#include "tensor.h"
#include "imgproc.h"
#include "container_vector.h"
#include "container_linked_list.h"
#include "detection/object_box.h"

typedef enum plate_color_t
{
    PLATE_COLOR_WHITE = 0,
    PLATE_COLOR_YELLOW,
    PLATE_COLOR_BLUE,
    PLATE_COLOR_BLACK,
    PLATE_COLOR_GREEN,
    PLATE_COLOR_UNKNOWN = -1,
} plate_color_t;

typedef enum char_type_t
{
    CHINESE,
    LETTER,
    LETTER_NUMS,
    INVALID
} char_type_t;

typedef struct plate_info_t
{
    object_box_t bbox;

    float landmark_reg[8];
    float landmark[8];

    plate_color_t plate_color;
    char plate_no[16];

    image_t plate_image;
} plate_info_t;

FUNCTION_IRAM plate_info_t plate_info_create_default();

typedef struct plate_info_node
{
    linked_node pointer;
    plate_info_t plate_info;
} plate_info_node;

/**
 * Create plate info node
 * @param value
 * @return
 */
FUNCTION_IRAM plate_info_node *plate_info_node_create(plate_info_t *value);

/**
 * plate image process
 * @param img
 * @param info
 * @param dst_w
 * @param dst_h
 * @return
 */
tensor_t plate_preprocess(tensor_t *img, plate_info_t info,  int dst_w, int dst_h);

#endif //CNET_PLATE_INFO_H
