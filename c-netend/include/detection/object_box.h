/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_OBJECT_BOX_H
#define CNET_OBJECT_BOX_H

#include "allocator.h"
#include "container_linked_list.h"
#include "tensor.h"
#include "imgproc.h"

typedef struct object_box_t
{
    float x_min;
    float y_min;
    float x_max;
    float y_max;

    float w;
    float h;

    float bbox_reg[4];

    float score;
    int classid;

    int anchor_x;
    int anchor_y;
    int anchor_n;
} object_box_t;

FUNCTION_IRAM object_box_t object_box_create_default();
FUNCTION_IRAM float object_box_intersection(object_box_t *a, object_box_t *b);
FUNCTION_IRAM float object_box_union(object_box_t *a, object_box_t *b);

typedef float (*object_box_iou_fun)(object_box_t*, object_box_t*);
FUNCTION_IRAM float object_box_iou_union(object_box_t *a, object_box_t *b);
FUNCTION_IRAM float object_box_iou_min(object_box_t *a, object_box_t *b);
FUNCTION_IRAM float object_box_iou_max(object_box_t *a, object_box_t *b);
void object_box_print(linked_node *node, void *args);

typedef struct object_box_node
{
    linked_node pointer;
    object_box_t box;
} object_box_node;

FUNCTION_IRAM int object_box_node_compare(object_box_node *a, object_box_node *b);
FUNCTION_IRAM object_box_node *object_box_node_create(object_box_t *value);

/**
 * do nms for objects, list is sored
 * @param objects_list
 * @param nms
 * @param object_box_iou. iou function
 */
FUNCTION_IRAM void object_boxes_nms(linked_list *objects_list, float nms, object_box_iou_fun object_box_iou);

/**
 * Draw object boxs
 * @param objects_list
 * @param image
 * @param r
 * @param g
 * @param b
 */
void object_boxes_draw(
        linked_list *objects_list,
        tensor_t *image,
        float r,
        float g ,
        float b);

/**
 * Draw object boxs to image, and save
 * @param object_list
 * @param image
 * @param file_name
 * @param file_type
 * @param r
 * @param g
 * @param b
 */
void object_boxes_save_to_image(
        linked_list *object_list,
        tensor_t *image,
        char * file_name,
        image_file_type_t file_type,
        float r,
        float g ,
        float b);

#endif //CNET_OBJECT_BOX_H
