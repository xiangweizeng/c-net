/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include <math.h>
#include "detection/yolo.h"
#include "option.h"
#include <container_linked_list.h>
#include <detection/object_box.h>

yolov3_t yolov3_create_default()
{
    yolov3_t yolov3 = {
        .num_class = 20,
        .num_box = 5,
        .nms_threshold = 0.45,
        .confidence_threshold  = 0.25,
        .anchors = tensor_create_default(),
        .anchors_scale = tensor_create_default(),
        .mask = tensor_create_default(),
        .mask_group_num = 0,
    };

    return yolov3;
}

static inline float sigmoid(float x) {
    return 1.f / (1.f + exp(-x));
}

int yolov3_forward(yolov3_t *yolo, vector_tensor_t *tensors, linked_list *output, option_t *opt) {

    linked_list objects = create_linked_list((linked_node_compare) object_box_node_compare);
    float *biases = (float *) yolo->anchors.data;
    int *mask = (int*)yolo->mask.data;
    float *anchors_scale = (float*)yolo->anchors_scale.data;
    for (size_t b = 0; b < tensors->length; b++) {
        tensor_t *bottom_top_tensor = &tensors->data[b];

        int w = bottom_top_tensor->d0;
        int h = bottom_top_tensor->d1;
        int channels = bottom_top_tensor->d2;

        const int channels_per_box = channels / yolo->num_box;
        // anchor coord + box score + num_class
        if (channels_per_box != 4 + 1 + yolo->num_class){
            destroy_linked_list(&objects);
            return CNET_STATUS_FAILED;
        }


        int mask_offset = b * yolo->num_box;
        int net_w = (int)(anchors_scale[b] * w);
        int net_h = (int)(anchors_scale[b] * h);

        for (int pp = 0; pp < yolo->num_box; pp++) {
            int p = pp * channels_per_box;

            int biases_index = mask[pp + mask_offset];
            const float bias_w = biases[biases_index * 2];
            const float bias_h = biases[biases_index * 2 + 1];

            const float *xptr = (float *) tensor_d2(bottom_top_tensor, p).data;
            const float *yptr = (float *) tensor_d2(bottom_top_tensor, p + 1).data;
            const float *wptr = (float *) tensor_d2(bottom_top_tensor, p + 2).data;
            const float *hptr = (float *) tensor_d2(bottom_top_tensor, p + 3).data;

            const float *box_score_ptr = (float *) tensor_d2(bottom_top_tensor, p + 4).data;

            // softmax class scores
            tensor_t scores = tensor_d2_range(bottom_top_tensor, p + 5, yolo->num_class);
             for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    // box score
                    float box_score = sigmoid(box_score_ptr[0]);

                    // find class index with max class score
                    int class_index = 0;
                    float class_score = 0.f;
                    for (int q = 0; q < yolo->num_class; q++) {
                        tensor_t m = tensor_d2(&scores, q);
                        float score = *((float *) m.data + m.d0 * i + j);
                        score = sigmoid(score);
                        if (score > class_score) {
                            class_index = q;
                            class_score = score;
                        }
                    }

                    float confidence = box_score * class_score;
                    if (confidence >= yolo->confidence_threshold) {

                        float bbox_cx = (j + sigmoid(xptr[0])) / w;
                        float bbox_cy = (i + sigmoid(yptr[0])) / h;
                        float bbox_w = exp(wptr[0]) * bias_w / net_w;
                        float bbox_h = exp(hptr[0]) * bias_h / net_h;

                        float bbox_xmin = bbox_cx - bbox_w * 0.5f;
                        float bbox_ymin = bbox_cy - bbox_h * 0.5f;
                        float bbox_xmax = bbox_cx + bbox_w * 0.5f;
                        float bbox_ymax = bbox_cy + bbox_h * 0.5f;

                        object_box_t obj_box = object_box_create_default();
                        obj_box.x_min = bbox_xmin;
                        obj_box.y_min = bbox_ymin;
                        obj_box.x_max = bbox_xmax;
                        obj_box.y_max = bbox_ymax;
                        obj_box.w = bbox_xmax - bbox_xmin + 1;
                        obj_box.h = bbox_ymax - bbox_ymin + 1;
                        obj_box.classid = class_index;
                        obj_box.score = confidence;

                        object_box_node *node = object_box_node_create(&obj_box);
                        linked_list_insert_sort(&objects, (linked_node *) node);
                    }

                    xptr++;
                    yptr++;
                    wptr++;
                    hptr++;

                    box_score_ptr++;
                }
            }
        }
    }

    object_boxes_nms(&objects, yolo->nms_threshold, object_box_iou_union);
    return CNET_STATUS_SUCCESS;
}