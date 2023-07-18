/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_YOLO_H
#define CNET_YOLO_H

#include "tensor.h"
#include "container_linked_list.h"
#include "option.h"

/**
 * yolov2_detection_output layer
 */
typedef struct yolov2_detection_output_t
{
    int num_class;
    int num_box;
    float confidence_threshold;
    float nms_threshold;
    tensor_t anchors;
} yolov2_t;

/**
 * yolo v2 forward
 * @param yolo
 * @param tensors
 * @param output
 * @param opt
 * @return
 */
int yolo2_forward(yolov2_t *yolo, vector_tensor_t *tensors, linked_list *output, option_t *opt);

/**
 * yolov3_detection_output layer
 */
typedef struct yolov3_detection_output_t
{
    int num_class;
    int num_box;
    float confidence_threshold;
    float nms_threshold;
    tensor_t anchors;
    tensor_t mask;
    tensor_t anchors_scale;
    int mask_group_num;
} yolov3_t;

/**
 * yolov3 crete default
 * @return
 */
yolov3_t yolov3_create_default();

/**
 * yolov3 forward
 * @param yolo
 * @param tensors
 * @param output, need release
 * @param opt
 * @return
 */
int yolov3_forward(yolov3_t *yolo, vector_tensor_t *tensors, linked_list *output, option_t *opt);

#endif //CNET_YOLO_H
