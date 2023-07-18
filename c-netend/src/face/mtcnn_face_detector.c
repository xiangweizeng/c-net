/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "face/mtcnn_face_detector.h"
#include "face/face_info.h"
#include "tensor_pixel.h"

void mtcnn_face_refine_object_box(linked_list *objects_list, int height, int width, int flag, float scale_side) {
    if (NULL == objects_list || 0 == objects_list->linked_size)
        return;

    linked_node *current = objects_list->head;
    for (; NULL != current; current = current->next) {
        object_box_t *bx = &((object_box_node *) current)->box;
        float bw = bx->w;
        float bh = bx->h;
        float x0 = bx->x_min + bx->bbox_reg[0] * bw;
        float y0 = bx->y_min + bx->bbox_reg[1] * bh;
        float x1 = bx->x_max + bx->bbox_reg[2] * bw;
        float y1 = bx->y_max + bx->bbox_reg[3] * bh;

        if (flag) {
            float w = x1 - x0 + 1;
            float h = y1 - y0 + 1;
            float w_side = (h > w) ? h : w;
            float h_side = w_side * scale_side;
            x0 = x0 + w * 0.5 - w_side * 0.5;
            y0 = y0 + h * 0.5 - h_side * 0.5;
            x1 = x0 + w_side - 1;
            y1 = y0 + h_side - 1;
        }
        bx->x_min = round(x0);
        bx->y_min = round(y0);
        bx->x_max = round(x1);
        bx->y_max = round(y1);

        if (bx->x_min < 0) bx->x_min = 0;
        if (bx->y_min < 0) bx->y_min = 0;
        if (bx->x_max > width) bx->x_max = width - 1;
        if (bx->y_max > height) bx->y_max = height - 1;

        bx->w = (bx->x_max - bx->x_min);
        bx->h = (bx->y_max - bx->y_min);
    }
}

void onet_face_detect_parse_object(
        object_box_t *origon_object_box,
        onet_parse_param_t *param,
        linked_list *face_infos_list) {

    float *score = (float *) param->prob->data;
    float *bbox = (float *) param->box->data;
    float *point = (float *) param->landmark->data;
    if (score[1] > param->threshold) {
        for (int c = 0; c < 4; c++) {
            origon_object_box->bbox_reg[c] = (float) bbox[c];
        }
        origon_object_box->score = (float) score[1];

        face_info_t face_info = face_info_create_default();
        face_info.bbox = *origon_object_box;

        for (int p = 0; p < 5; p++) {
            face_info.landmark[2 * p] =
                    origon_object_box->x_min + (origon_object_box->x_max - origon_object_box->x_min) * point[p];
            face_info.landmark[2 * p + 1] =
                    origon_object_box->y_min + (origon_object_box->y_max - origon_object_box->y_min) * point[p + 5];
        }

        face_info_node *node = face_info_node_create(&face_info);
        linked_list_insert_sort(face_infos_list, (linked_node *) node);
    }
}

linked_list mtcnn_face_detector_run(mtcnn_detector_t *detector, tensor_t *image)
{
    int img_w = image->d0;
    int img_h = image->d1;

    const float mean_vals[3] = {127.5f, 127.5f, 127.5f};
    const float norm_vals[3] = {0.0078125f, 0.0078125f, 0.0078125f};
    tensor_substract_mean_normalize(image, mean_vals, norm_vals);

    linked_list detect_objects = run_pnet(detector->pnet, &detector->pnet_param, image);
    object_boxes_nms(&detect_objects, detector->pnet_param.nms, object_box_iou_union);
    mtcnn_face_refine_object_box(&detect_objects, img_h, img_w, 1, detector->side_scale);

    detect_objects = run_rnet(detector->rnet, &detector->rnet_param, image, &detect_objects);
    object_boxes_nms(&detect_objects, detector->rnet_param.nms, object_box_iou_union);
    mtcnn_face_refine_object_box(&detect_objects, img_h, img_w, 1, detector->side_scale);

    linked_list face_infos_list = run_onet(detector->onet, &detector->onet_param, image, onet_face_detect_parse_object, &detect_objects);
    object_boxes_nms(&face_infos_list, detector->onet_param.nms, object_box_iou_min);
    mtcnn_face_refine_object_box(&face_infos_list, img_h, img_w, 0, detector->side_scale);

    return face_infos_list;
}