/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_MTCNN_PLATE_DETECTOR_H
#define CNET_MTCNN_PLATE_DETECTOR_H

#include "detection/mtcnn.h"

/**
 * plate refine object box
 * @param objects_list
 * @param height
 * @param width
 * @param flag  if flag != 0 , pad square
 * @param scale_side  side_h * side_w * scale_side
 */
FUNCTION_IRAM void mtcnn_plate_refine_object_box(
        linked_list * objects_list,
        int height,
        int width,
        int flag,
        float scale_side);


/**
 * parse current object_box to plate_infos_list
 * @param origin_object_box
 * @param param
 * @param plate_infos_list
 */
FUNCTION_IRAM void onet_plate_detect_parse_object(
        object_box_t *origin_object_box,
        onet_parse_param_t *param ,
        linked_list *plate_infos_list);

/**
 * Run detector, return plate_infos
 * @param detector
 * @param image
 * @return plate_info_t lineded list, need to destory
 */
linked_list mtcnn_plate_detector_run(mtcnn_detector_t *detector, tensor_t *image);



#endif //CNET_MTCNN_PLATE_DETECTOR_H
