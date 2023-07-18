

/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "tensor_pixel.h"
#include "models/plate/lpr-opt.h"
#include "models/plate/lpc-opt.h"
#include <lpr/lpr_recognizer.h>
#include <lpr/lpc_recognizer.h>

void test_lpr(image_t *bgr) {
    tensor_t image = tensor_from_pixels_resize(bgr->buf, PIXEL_RGB2BGR, bgr->w, bgr->h, 128, 32, NULL);;

    lpr_recognizer_t lpr;
    lpr_recognizer_create(&lpr, &network_lpr_opt, lpr_opt_blob_input_data, lpr_opt_blob__permute2_reshape_fc1x_fc1x);

    plate_info_t plate_info;
    lpr_recognizer_predict_number(&lpr, &image, &plate_info);
    printf("%s\n", plate_info.plate_no);
    tensor_release(&image);

    lpr_recognizer_release(&lpr);
}


void test_lpc(image_t *bgr) {
    tensor_t image = tensor_from_pixels_resize(bgr->buf, PIXEL_RGB2BGR, bgr->w, bgr->h, 110, 22, NULL);;

    lpc_recognizer_t lpr;
    lpc_recognizer_create(&lpr, &network_lpc_opt, lpc_opt_blob_data_data, lpc_opt_blob_prob_prob);

    plate_color_t color = lpc_recognizer_predict_color(&lpr, &image);
    printf("%s\n", lpc_recognizer_get_color_name(color));
    tensor_release(&image);

    lpc_recognizer_release(&lpr);
}



int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s  [image_path]\n", argv[0]);
        return -1;
    }

    char* image_path = argv[1];
    image_t image_data = load_image(image_path);
    if(image_data.w == 0){
        printf("read image : %s failed\n", argv[1]);
        return -1;
    }

    test_lpr(&image_data);
    test_lpc(&image_data);
    image_free(&image_data);

    return 0;
}

