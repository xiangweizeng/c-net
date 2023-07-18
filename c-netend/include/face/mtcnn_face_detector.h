/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_MTCNN_FACE_DETECTOR_H
#define CNET_MTCNN_FACE_DETECTOR_H

#include "detection/mtcnn.h"

/**
 * Face refine object box
 * @param objects_list
 * @param height
 * @param width
 * @param flag  if flag != 0 , pad square
 * @param scale_side  side_h * side_w * scale_side
 */
FUNCTION_IRAM void mtcnn_face_refine_object_box(
        linked_list * objects_list,
        int height,
        int width,
        int flag,
        float scale_side);

/**
 * Parse current object_box to face_infos_list
 * @param origin_object_box
 * @param param
 * @param face_infos_list
 */
FUNCTION_IRAM void onet_face_detect_parse_object(
        object_box_t *orign_object_box,
        onet_parse_param_t *param ,
        linked_list *face_infos_list);

/**
 * Run detector, return face
 * @param detector
 * @param image
 * @return plate_info_t linked list, need to destroy
 */
linked_list mtcnn_face_detector_run(mtcnn_detector_t *detector, tensor_t *image);

#endif //CNET_MTCNN_FACE_DETECTOR_H
