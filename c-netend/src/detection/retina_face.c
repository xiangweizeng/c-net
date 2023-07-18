/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <math.h>
#include "detection/retina_face.h"
#include "detection/object_box.h"
#include "lpr/plate_info.h"
#include "face/face_info.h"

VECTOR_IMPL(box_t)

static vector_box_t create_anchor(int w, int h) {

    static float steps[4] = {8, 16, 32, 64};

    int feature_map[4][2] = {
            {ceilf(h / 8.f),  ceilf(w / 8.f)},
            {ceilf(h / 16.f), ceilf(w / 16.f)},
            {ceilf(h / 32.f), ceilf(w / 32.f)},
            {ceilf(h / 64.f), ceilf(w / 64.f)},
    };

    static int min_sizes[4][4] = {
            {4, 10,  16,  24},
            {3, 32,  48},
            {3, 64,  96},
            {4, 128, 192, 256}
    };

    VECTOR_DECLARE(box_t, anchors) = VECTOR_CREATE(box_t, 0);
    for (int k = 0; k < 4; ++k) {
        int *min_size = min_sizes[k];
        for (int i = 0; i < feature_map[k][0]; ++i) {
            for (int j = 0; j < feature_map[k][1]; ++j) {
                for (int l = 1; l < min_size[0]; ++l) {
                    float s_kx = min_size[l] * 1.0 / w;
                    float s_ky = min_size[l] * 1.0 / h;
                    float cx = (j + 0.5) * steps[k] / w;
                    float cy = (i + 0.5) * steps[k] / h;
                    box_t anchor = {cx, cy, s_kx, s_ky};
                    anchors.push_element(&anchors, &anchor);
                }
            }
        }
    }

    return anchors;
}

static vector_box_t create_anchor_retina_face(int w, int h) {
    float steps[] = {8, 16, 32};

    int feature_map[4][2] = {
            {ceilf(h / 16.f), ceilf(w / 16.f)},
            {ceilf(h / 32.f), ceilf(w / 32.f)},
            {ceilf(h / 64.f), ceilf(w / 64.f)},
    };

    static int min_sizes[3][3] = {
            {3, 10,  20},
            {3, 32,  64},
            {3, 128, 256},
    };

    VECTOR_DECLARE(box_t, anchors) = VECTOR_CREATE(box_t, 0);
    for (int k = 0; k < 3; ++k) {
        int *min_size = min_sizes[k];
        for (int i = 0; i < feature_map[k][0]; ++i) {
            for (int j = 0; j < feature_map[k][1]; ++j) {
                for (int l = 1; l < min_size[0]; ++l) {
                    float s_kx = min_size[l] * 1.0f / w;
                    float s_ky = min_size[l] * 1.0f / h;
                    float cx = (j + 0.5f) * steps[k] / w;
                    float cy = (i + 0.5f) * steps[k] / h;
                    box_t anchor = {cx, cy, s_kx, s_ky};
                    anchors.push_element(&anchors, &anchor);
                }
            }
        }
    }

    return anchors;
}


retina_face_t retina_face_create(retina_face_config config)
{
    retina_face_t face;
    face.threshold = config.threshold;
    face.land_type = config.land_type;
    face.nms = config.nms;

    if (config.retain_face){
        face.anchors = create_anchor_retina_face(config.width, config.height);
    }else{
        face.anchors = create_anchor(config.width, config.height);
    }

    return face;
}


void retina_face_release(retina_face_t *retina_face)
{
    if(NULL != retina_face){
        retina_face->anchors.free_data(&retina_face->anchors);
    }
}

plate_info_node * retina_face_parse_4p(
        box_t *anchor,
        box_t *location_box,
        float score,
        const float *land_ptr,
        int land_num) {

    /// parse object info
    plate_info_t plate_info = plate_info_create_default();
    plate_info.bbox.x_min = ((*location_box).cx - (*location_box).sx / 2);
    plate_info.bbox.y_min = ((*location_box).cy - (*location_box).sy / 2);
    plate_info.bbox.x_max = ((*location_box).cx + (*location_box).sx / 2);
    plate_info.bbox.y_max = ((*location_box).cy + (*location_box).sy / 2);
    plate_info.bbox.score = score;
    plate_info.bbox.w = plate_info.bbox.x_max - plate_info.bbox.x_min;
    plate_info.bbox.h = plate_info.bbox.y_max - plate_info.bbox.y_min;

    /// landmarks
    for (int j = 0; j < land_num; ++j) {
        plate_info.landmark[j * 2] = ((*anchor).cx + *(land_ptr + (j << 1)) * 0.1 * (*anchor).sx);
        plate_info.landmark[j * 2 + 1] = ((*anchor).cy + *(land_ptr + (j << 1) + 1) * 0.1 * (*anchor).sy);
    }

    plate_info_node *node = plate_info_node_create(&plate_info);
    return node;
}

face_info_node * retina_face_parse_5p(
        box_t *anchor,
        box_t *location_box,
        float score,
        const float *land_ptr,
        int land_num) {

    /// parse object info
    face_info_t face_info = face_info_create_default();
    face_info.bbox.x_min = ((*location_box).cx - (*location_box).sx / 2);
    face_info.bbox.y_min = ((*location_box).cy - (*location_box).sy / 2);
    face_info.bbox.x_max = ((*location_box).cx + (*location_box).sx / 2);
    face_info.bbox.y_max = ((*location_box).cy + (*location_box).sy / 2);
    face_info.bbox.score = score;
    face_info.bbox.w = face_info.bbox.x_max - face_info.bbox.x_min;
    face_info.bbox.h = face_info.bbox.y_max - face_info.bbox.y_min;

    /// landmark
    for (int j = 0; j < land_num; ++j) {
        face_info.landmark[j * 2] = ((*anchor).cx + *(land_ptr + (j << 1)) * 0.1 * (*anchor).sx);
        face_info.landmark[j * 2 + 1] = ((*anchor).cy + *(land_ptr + (j << 1) + 1) * 0.1 * (*anchor).sy);
    }

    face_info_node *node = face_info_node_create(&face_info);
    return node;
}

void retina_face_forward(retina_face_t *retina_face, vector_tensor_t *tensors, linked_list *output, option_t *opt)
{
    *output = create_linked_list((linked_node_compare)object_box_node_compare);
    tensor_t location = tensors->data[0];
    tensor_t classes = tensors->data[1];
    tensor_t landmark = tensors->data[2];

    float *location_ptr = (float *) location.data;
    float *classes_ptr = (float *) classes.data;
    float *landmark_ptr = (float *) landmark.data;
    int landmark_num = retina_face->land_type == 0 ? 4 : 5;

    for (int i = 0; i < retina_face->anchors.current_size; ++i) {
        if (*(classes_ptr + 1) > retina_face->threshold) {
            box_t anchor = retina_face->anchors.data[i];

            /// location and confidence
            box_t location_box;
            location_box.cx = anchor.cx + *location_ptr * 0.1 * anchor.sx;
            location_box.cy = anchor.cy + *(location_ptr + 1) * 0.1 * anchor.sy;
            location_box.sx = anchor.sx * exp(*(location_ptr + 2) * 0.2);
            location_box.sy = anchor.sy * exp(*(location_ptr + 3) * 0.2);
            float score = *(classes_ptr + 1);

            if(retina_face->land_type == 0){
                linked_node *node = (linked_node *) retina_face_parse_4p(
                        &anchor, &location_box, score, landmark_ptr, landmark_num);
                linked_list_insert_sort(output, node);
            } else{

                linked_node *node = (linked_node *) retina_face_parse_5p(
                        &anchor, &location_box, score, landmark_ptr, landmark_num);
                linked_list_insert_sort(output, node);
            }
        }

        location_ptr += 4;
        classes_ptr += 2;
        landmark_ptr += landmark_num * 2;
    }

    object_boxes_nms(output, retina_face->nms, object_box_iou_union);
}