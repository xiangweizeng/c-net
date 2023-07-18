

/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "tensor_pixel.h"
#include "models/face//mobilefacenet-opt.h"
#include <face/arc_face.h>

void test_lpr(image_t *a, image_t *b) {

    arc_face_t arc_face = {
            .net = &network_mobilefacenet_opt,
            .input_blob_index = mobilefacenet_opt_blob_data_data,
            .vector_blob_index = mobilefacenet_opt_blob_pre_fc1_fc1
    };

    arc_face_create(&arc_face);

    tensor_t image = tensor_from_pixels_resize(a->buf, PIXEL_RGB, a->w, a->h, 112, 112, NULL);
    vector_float feature1 = arc_face_get_face_feature(&arc_face, &image);
    tensor_release(&image);

    image = tensor_from_pixels_resize(b->buf, PIXEL_RGB, b->w, b->h, 112, 112, NULL);
    vector_float feature2 = arc_face_get_face_feature(&arc_face, &image);
    tensor_release(&image);
    arc_face_release(&arc_face);

    printf("similar:%f", face_feature_calc_similar(feature1.data, feature2.data, feature2.current_size));
    feature1.free_data(&feature1);
    feature2.free_data(&feature2);
}


int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s  [image_a_path] [image_b_path]\n", argv[0]);
        return -1;
    }

    char* image_path = argv[1];
    image_t image_a_data = load_image(image_path);
    if(image_a_data.w == 0){
        printf("read image : %s failed\n", argv[1]);
        return -1;
    }

    image_path = argv[2];
    image_t image_b_data = load_image(image_path);
    if(image_b_data.w == 0){
        printf("read image : %s failed\n", argv[2]);
        return -1;
    }

    test_lpr(&image_a_data, &image_b_data);
    image_free(&image_a_data);
    image_free(&image_b_data);

    return 0;
}

