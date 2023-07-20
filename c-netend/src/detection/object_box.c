/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "detection/object_box.h"
#include "tensor_pixel.h"

#include <stdio.h>

object_box_t object_box_create_default() {
    object_box_t box;
    memset(&box, 0, sizeof(box));
    box.classid = -1;
    box.anchor_x = -1;
    box.anchor_y = -1;
    box.anchor_n = -1;
    return box;
}

float box_overlap(float x1, float w1, float x2, float w2) {
    float left = x1 > x2 ? x1 : x2;
    float r1 = x1 + w1;
    float r2 = x2 + w2;
    float right = r1 < r2 ? r1 : r2;
    return right - left;
}

float object_box_intersection(object_box_t *a, object_box_t *b) {
    float w = box_overlap(a->x_min, a->w, b->x_min, b->w);
    float h = box_overlap(a->y_min, a->h, b->y_min, b->h);
    if (w < 0 || h < 0)
        return CNET_STATUS_SUCCESS;
    float area = w * h;
    return area;
}

float object_box_union(object_box_t *a, object_box_t *b) {
    float i = object_box_intersection(a, b);
    float u = a->w * a->h + b->w * b->h - i;
    return u;
}

float object_box_iou_union(object_box_t *a, object_box_t *b) {
    return object_box_intersection(a, b) / object_box_union(a, b);
}

float object_box_iou_min(object_box_t *a, object_box_t *b) {
    float a_area = (a->w * a->h);
    float b_area = (b->w * b->h);
    return object_box_intersection(a, b) / (a_area < b_area ? a_area : b_area);
}

float object_box_iou_max(object_box_t *a, object_box_t *b) {
    float a_area = (a->w * a->h);
    float b_area = (b->w * b->h);
    return object_box_intersection(a, b) / (a_area > b_area ? a_area : b_area);
}

int object_box_node_compare(object_box_node *a, object_box_node *b) {
    if (NULL == a || NULL == b)
        return CNET_STATUS_SUCCESS;

    float diff = a->box.score - b->box.score;
    if (diff > 0)
        return 1;
    else if (diff < 0)
        return CNET_STATUS_FAILED;
    return CNET_STATUS_SUCCESS;
}

object_box_node *object_box_node_create(object_box_t *value) {
    object_box_node *node = (object_box_node *) fast_malloc(sizeof(object_box_node));
    node->pointer = create_linked_node();
    node->box = *value;
    return node;
}

void object_box_print(linked_node *node, void *args) {
    object_box_node *box = (object_box_node *) node;
    if (NULL != box) {
        printf("%d=%f: %f %f %f %f\n", box->box.classid, box->box.score, box->box.x_min, box->box.y_min, box->box.x_max,
               box->box.x_max);
    }

}


void object_boxes_nms(linked_list *objects_list, float nms, object_box_iou_fun object_box_iou) {
    if (NULL == objects_list || nms <= 0)
        return;

    object_box_node *current = (object_box_node *) objects_list->head;
    while (NULL != current) {
        object_box_node *next_node = (object_box_node *) (current->pointer.next);
        while (NULL != next_node) {

            object_box_node *free_node = NULL;
            if (current->box.classid == next_node->box.classid &&
                object_box_iou(&current->box, &next_node->box) > nms) {
                free_node = (object_box_node *) next_node;
            }

            next_node = (object_box_node *) (next_node->pointer.next);
            if (NULL != free_node) {
                linked_list_delete_node(objects_list, &free_node->pointer);
                fast_free(free_node);
            }
        }
        current = (object_box_node *) (current->pointer.next);
    }
}

void object_boxes_draw(linked_list *objects_list, tensor_t *image, float r, float g, float b) {

    object_box_node *current = (object_box_node *) objects_list->head;
    while (NULL != current) {
        draw_rgb_image_box_width(
                *image,
                current->box.x_min, current->box.y_min,
                current->box.x_max, current->box.y_max, 2,
                r, g, b);
        current = (object_box_node *) (current->pointer.next);
    }
}

void object_boxes_save_to_image(
        linked_list *object_list,
        tensor_t *image,
        char *file_name,
        image_file_type_t file_type,
        float r, float g, float b) {

    image_t to_save_image;
    image_create(&to_save_image, image->d0, image->d1, image->d2, PIXEL_BGR_DATA);

    object_boxes_draw(object_list, image, r, g, b);

    tensor_to_pixels(image, to_save_image.buf, PIXEL_BGR);
    save_image(to_save_image, file_type, file_name);

    image_free(&to_save_image);
}

