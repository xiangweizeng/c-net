/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_FACE_INFO_H
#define CNET_FACE_INFO_H

#include "imgproc.h"
#include "detection/object_box.h"

/**
 * Face info
 */
typedef struct face_info_t
{
    object_box_t bbox;

    float landmark_reg[10];
    float landmark[10];

    image_t plate_image;
} face_info_t;

FUNCTION_IRAM face_info_t face_info_create_default();

/**
 * Face info node
 */
typedef struct face_info_node
{
    linked_node pointer;
    face_info_t face_info;
} face_info_node;

/**
 * Create face info node
 * @param value
 * @return
 */
FUNCTION_IRAM face_info_node *face_info_node_create(face_info_t *value);

/**
 * Face feature normalize
 * @param feature
 * @param size
 */
FUNCTION_IRAM void face_feature_normalize(float* feature, int size);


/**
 * Calc face feature similar
 * @param feature1
 * @param feature2
 * @param size
 * @return
 */
FUNCTION_IRAM float face_feature_calc_similar(float* feature1, float*feature2, int size);

/**
 * Face image process
 * @param img
 * @param info
 * @return
 */
FUNCTION_IRAM tensor_t plate_image_preprocess(tensor_t *img, face_info_t info);


#endif //CNET_FACE_INFO_H
