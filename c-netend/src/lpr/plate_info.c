/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "lpr/plate_info.h"
#include "detection/affine_matrix.h"

plate_info_t plate_info_create_default() {
    plate_info_t plate_info;
    memset(&plate_info, 0, sizeof(plate_info));
    plate_info.bbox = object_box_create_default();
    return plate_info;
}

plate_info_node *plate_info_node_create(plate_info_t *value) {
    plate_info_node *node = (plate_info_node *) fast_malloc(sizeof(plate_info_node));
    node->pointer = create_linked_node();
    node->plate_info = *value;
    return node;
}

tensor_t plate_preprocess(tensor_t *img, plate_info_t info,  int dst_w, int dst_h) {

    float dst[8];
    float src[8];

    dst[0] = 0;
    dst[4] = 0;
    dst[1] = dst_w;
    dst[5] = 0;
    dst[2] = dst_w;
    dst[6] = dst_h;
    dst[3] = 0;
    dst[7] = dst_h;

    src[0] = info.landmark[0];
    src[4] = info.landmark[1];
    src[1] = info.landmark[2];
    src[5] = info.landmark[3];
    src[2] = info.landmark[4];
    src[6] = info.landmark[5];
    src[3] = info.landmark[6];
    src[7] = info.landmark[7];

    float M[6];
    get_affine_matrix_from_4p(src, dst, M);
    return warp_affine_matrix(img, M, dst_w, dst_h);
}
