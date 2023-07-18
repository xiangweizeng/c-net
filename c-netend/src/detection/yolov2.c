/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/


#include <math.h>
#include "detection/yolo.h"
#include "tensor_operation.h"
#include "option.h"
#include <container_linked_list.h>
#include <detection/object_box.h>

static inline float sigmoid(float x) {
    return 1.f / (1.f + exp(-x));
}

int yolo2_forward(yolov2_t *yolo, vector_tensor_t *tensors, linked_list *output, option_t *opt) {

    *output = create_linked_list((linked_node_compare) object_box_node_compare);
    for (size_t b = 0; b < tensors->length; b++) {
        tensor_t *bottom_top_tensor = &tensors->data[b];

        int w = bottom_top_tensor->d0;
        int h = bottom_top_tensor->d1;
        int channels = bottom_top_tensor->d2;

        const int channels_per_box = channels / yolo->num_box;
        // anchor coord + box score + num_class
        if (channels_per_box != 4 + 1 + yolo->num_class)
            return CNET_STATUS_FAILED;

        float *biases = (float *) yolo->anchors.data;
        for (int pp = 0; pp < yolo->num_box; pp++) {
            int p = pp * channels_per_box;

            const float bias_w = biases[pp * 2];
            const float bias_h = biases[pp * 2 + 1];

            const float *xptr = (float *) tensor_d2(bottom_top_tensor, p).data;
            const float *yptr = (float *) tensor_d2(bottom_top_tensor, p + 1).data;
            const float *wptr = (float *) tensor_d2(bottom_top_tensor, p + 2).data;
            const float *hptr = (float *) tensor_d2(bottom_top_tensor, p + 3).data;

            const float *box_score_ptr = (float *) tensor_d2(bottom_top_tensor, p + 4).data;

            // softmax class scores
            tensor_t scores = tensor_d2_range(bottom_top_tensor, p + 5, yolo->num_class);
            tensor_softmax(0, &scores, opt);

            for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    // region box
                    float bbox_cx = (j + sigmoid(xptr[0])) / w;
                    float bbox_cy = (i + sigmoid(yptr[0])) / h;
                    float bbox_w = exp(wptr[0]) * bias_w / w;
                    float bbox_h = exp(hptr[0]) * bias_h / h;

                    float bbox_xmin = bbox_cx - bbox_w * 0.5f;
                    float bbox_ymin = bbox_cy - bbox_h * 0.5f;
                    float bbox_xmax = bbox_cx + bbox_w * 0.5f;
                    float bbox_ymax = bbox_cy + bbox_h * 0.5f;

                    // box score
                    float box_score = sigmoid(box_score_ptr[0]);

                    // find class index with max class score
                    int class_index = 0;
                    float class_score = 0.f;
                    for (int q = 0; q < yolo->num_class; q++) {
                        tensor_t m = tensor_d2(&scores, q);
                        float score = *((float *) m.data + m.d0 * i + j);
                        if (score > class_score) {
                            class_index = q;
                            class_score = score;
                        }
                    }

                    float confidence = box_score * class_score;
                    if (confidence >= yolo->confidence_threshold) {

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
                        linked_list_insert_sort(output, (linked_node *) node);
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

    object_boxes_nms(output, yolo->nms_threshold, object_box_iou_union);
    return CNET_STATUS_SUCCESS;
}