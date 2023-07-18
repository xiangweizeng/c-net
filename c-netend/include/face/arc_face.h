/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_ARC_FACE_H
#define CNET_ARC_FACE_H

#include <stdio.h>
#include <pthreadpool.h>
#include "face_info.h"
#include "container_vector.h"
#include "network.h"

/**
 * Arc face
 */
typedef struct arc_face_t{
    network_t *net;
    size_t input_blob_index;
    size_t vector_blob_index;
}arc_face_t;

/**
 * Create arc face
 * @param arc_face
 * @return
 */
int arc_face_create(arc_face_t* arc_face);

/**
 * Release arcface
 * @param arc_face
 */
void arc_face_release(arc_face_t *arc_face);

/**
 * Get vector from face and origin image
 * @param arc_face
 * @param image
 * @return
 */
vector_float arc_face_get_face_feature(
        arc_face_t *arc_face,
        tensor_t *image);

#endif //CNET_ARC_FACE_H
