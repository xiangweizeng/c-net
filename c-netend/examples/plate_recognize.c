/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "models/plate/lpr-opt.h"
#include "models/plate/lpc-opt.h"
#include "models/plate/plate-opt.h"

#include "lpr/lpr_recognizer.h"
#include "lpr/lpc_recognizer.h"
#include "imgproc.h"

#include "detection/retina_face.h"
#include "lpr/plate_info.h"
#include "tensor_pixel.h"

void save_tensor_to_image(tensor_t *image, char *file_name, image_file_type_t file_type) {
    image_t to_save_image;
    image_create(&to_save_image, image->d0, image->d1, image->d2, PIXEL_BGR_DATA);

    tensor_to_pixels(image, to_save_image.buf, PIXEL_BGR);
    save_image(to_save_image, file_type, file_name);

    image_free(&to_save_image);
}


int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s  [image_path]\n", argv[0]);
        return -1;
    }

    image_t image_data = load_image(argv[1]);
    if(image_data.w == 0){
        printf("read image : %s failed\n", argv[1]);
        return -1;
    }

    int img_w = image_data.w;
    int img_h = image_data.h;
    const float mean_values[3] = { 104.f, 117.f, 123.f };
    tensor_t input = tensor_from_pixels_resize(image_data.buf, PIXEL_RGB2BGR, image_data.w, image_data.h, 320, 240, NULL);
    tensor_substract_mean_normalize(&input, mean_values, NULL);
    tensor_t draw_image = tensor_from_pixels(image_data.buf, PIXEL_RGB2BGR, image_data.w, image_data.h, NULL);
    image_free(&image_data);

    /**
     * set up run operations
     */
    option_t opt = option_get_instance();
    network_setup_run_operations(&network_plate_opt, NULL);

    /**
     * run network
     */
    session_t session = session_create(&network_plate_opt);
    session_set_input(&session, plate_opt_blob_input0_input0, input);
    tensor_release(&input);
    session_run(&session);

    /**
     * get tensors
     */
    vector_tensor_t outputs = vector_tensor_create_default(3);
    outputs.data[0] = session_get_output(&session, plate_opt_blob_output0_output0);
    outputs.data[1] = session_get_output(&session, plate_opt_blob_445_530);
    outputs.data[2] = session_get_output(&session, plate_opt_blob_444_529);
    session_release(&session);
    network_teardown_run_operations(&network_plate_opt, &opt);

    retina_face_config config;
    config.width = 320;
    config.height = 240;
    config.nms = 0.4;
    config.threshold = 0.4;
    config.retain_face = 0;
    config.land_type = 0;

    /**
     * forward retain_face
     */
    retina_face_t face = retina_face_create(config);
    linked_list detections = create_linked_list(NULL);
    retina_face_forward(&face, &outputs, &detections, NULL);
    retina_face_release(&face);
    vector_tensor_free(&outputs);
    option_uninit();

    lpr_recognizer_t lpr;
    lpr_recognizer_create(&lpr, &network_lpr_opt, lpr_opt_blob_input_data, lpr_opt_blob__permute2_reshape_fc1x_fc1x);

    lpc_recognizer_t lpc;
    lpc_recognizer_create(&lpc, &network_lpc_opt, lpc_opt_blob_data_data, lpc_opt_blob_prob_prob);

    linked_node *current = detections.head;
    while (current != NULL) {

        plate_info_t *plate_info = &((plate_info_node *) current)->plate_info;

        plate_info->bbox.x_min= plate_info->bbox.x_min * img_w;
        plate_info->bbox.y_min = plate_info->bbox.y_min * img_h;
        plate_info->bbox.x_max = plate_info->bbox.x_max * img_w;
        plate_info->bbox.y_max = plate_info->bbox.y_max * img_h;

        plate_info->landmark[0] = plate_info->landmark[0]*img_w;
        plate_info->landmark[1] = plate_info->landmark[1]*img_h;
        plate_info->landmark[2] = plate_info->landmark[2]*img_w;
        plate_info->landmark[3] = plate_info->landmark[3]*img_h;
        plate_info->landmark[4] = plate_info->landmark[4]*img_w;
        plate_info->landmark[5] = plate_info->landmark[5]*img_h;
        plate_info->landmark[6] = plate_info->landmark[6]*img_w;
        plate_info->landmark[7] = plate_info->landmark[7]*img_h;

        tensor_t lpr_image = plate_preprocess(&draw_image, *plate_info, 128, 32);
        lpr_recognizer_predict_number(&lpr, &lpr_image, plate_info);
        save_tensor_to_image(&lpr_image, "lpr_image-image222", IMAGE_FILE_TYPE_PNG);
        tensor_release(&lpr_image);

        tensor_t lpc_image = plate_preprocess(&draw_image, *plate_info, 110, 22);
        plate_color_t color = lpc_recognizer_predict_color(&lpc, &lpc_image);
        save_tensor_to_image(&lpc_image, "lpc_image-image222", IMAGE_FILE_TYPE_PNG);
        tensor_release(&lpc_image);

        printf("%s-%s\n", lpc_recognizer_get_color_name(color), plate_info->plate_no);

        for (int i = 0; i < 4; i++) {
            draw_rgb_image_point_width(
                    draw_image,
                    plate_info->landmark[2 * i],
                    plate_info->landmark[2 * i + 1],
                    5, 0, 255, 0);
        }
        current = current->next;
    }

    object_boxes_save_to_image(&detections, &draw_image, "plate_test", IMAGE_FILE_TYPE_PNG, 255, 0, 0);
    linked_list_for_each_forward(&detections, object_box_print, NULL);

    lpr_recognizer_release(&lpr);
    lpc_recognizer_release(&lpc);
    destroy_linked_list(&detections);
    tensor_release(&draw_image);

    return 0;
}