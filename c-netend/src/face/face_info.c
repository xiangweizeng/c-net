/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <math.h>
#include "face/face_info.h"
#include "detection/affine_matrix.h"

face_info_t face_info_create_default() {
    face_info_t face_info;
    memset(&face_info, 0, sizeof(face_info));
    face_info.bbox = object_box_create_default();
    return face_info;
}

face_info_node *face_info_node_create(face_info_t *value) {
    face_info_node *node = (face_info_node *) fast_malloc(sizeof(face_info_node));
    node->pointer = create_linked_node();
    node->face_info = *value;
    return node;
}

void face_feature_normalize(float *feature, int size) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += feature[i] * feature[i];
    }

    sum = sqrt(sum);
    for (int i = 0; i < size; i++) {
        feature[i] /= sum;
    }
}

float face_feature_calc_similar(float *feature1, float *feature2, int size) {
    float sim = 0.0;
    for (int i = 0; i < size; i++)
        sim += feature1[i] * feature2[i];
    return sim;
}

tensor_t plate_image_preprocess(tensor_t *img, face_info_t info) {
    int image_w = 112; //96 or 112
    int image_h = 112;

    float dst[10] = {30.2946, 65.5318, 48.0252, 33.5493, 62.7299,
                     51.6963, 51.5014, 71.7366, 92.3655, 92.2041};

    if (image_w == 112)
        for (int i = 0; i < 5; i++)
            dst[i] += 8.0;

    float src[10];
    for (int i = 0; i < 5; i++) {
        src[i] = info.landmark[2 * i];
        src[i + 5] = info.landmark[2 * i + 1];
    }

    float M[6];
    get_affine_matrix_from_5p(src, dst, M);
    return warp_affine_matrix(img, M, image_w, image_h);
}
