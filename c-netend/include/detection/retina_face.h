/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef C_NET_RETINA_FACE_H
#define C_NET_RETINA_FACE_H

#include "container_vector.h"
#include "container_linked_list.h"
#include "tensor.h"
#include "option.h"

/// anchor box
typedef struct box{
    float cx;
    float cy;
    float sx;
    float sy;
}box_t;
VECTOR_DEFINE(box_t)

/// Retina face
typedef struct retina_face{
    float threshold;
    float nms;
    int land_type; /// 0 face, 1 plate
    VECTOR_DECLARE(box_t, anchors);
}retina_face_t;

/// Retina face config
typedef struct retina_face_config{
    float threshold;
    float nms;
    int retain_face;
    int land_type; /// 0 face, 1 plate
    int width;
    int height;
}retina_face_config_t;

/**
 * Create retina face
 * @param config
 * @return
 */
retina_face_t retina_face_create(retina_face_config_t config);

/**
 * Release retina face
 * @param retina_face
 */
void retina_face_release(retina_face_t *retina_face);

/**
 * Forward retina face
 * @param retina_face
 * @param tensors
 * @param output
 * @param opt
 */
void retina_face_forward(retina_face_t *retina_face, vector_tensor_t *tensors, linked_list *output, option_t *opt);

#endif //C_NET_RETINA_FACE_H
