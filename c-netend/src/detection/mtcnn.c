/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <math.h>

#include "tensor_pixel.h"
#include "tensor_operation.h"
#include "detection/object_box.h"
#include "detection/mtcnn.h"

void pnet_parse_object(pnet_param_t *param, tensor_t *prob, tensor_t *box, float scale, linked_list *objects_list) {
    int feature_map_h = prob->d1;
    int feature_map_w = prob->d0;
    float inv_scale = 1.0f / scale;

    float *prob_data = (float *) tensor_d2(prob, 1).data;
    float *reg0 = (float *) tensor_d2(box, 0).data;
    float *reg1 = (float *) tensor_d2(box, 1).data;
    float *reg2 = (float *) tensor_d2(box, 2).data;
    float *reg3 = (float *) tensor_d2(box, 3).data;
    for (int h = 0; h < feature_map_h; h++) {
        for (int w = 0; w < feature_map_w; w++) {
            float score = *prob_data;
            if (score > param->threshold_prob) {
                object_box_t bbox = object_box_create_default();
                bbox.score = score;
                bbox.x_min = round((w * param->stride + 1) * inv_scale);
                bbox.y_min = round((h * param->stride + 1) * inv_scale);
                bbox.x_max = round((w * param->stride + param->cell_width + 1) * inv_scale);
                bbox.y_max = round((h * param->stride + param->cell_height + 1) * inv_scale);
                bbox.anchor_x = w;
                bbox.anchor_y = h;
                bbox.anchor_n = 0;
                bbox.classid = 1;

                bbox.w = bbox.x_max - bbox.x_min + 1;
                bbox.h = bbox.y_max - bbox.y_min + 1;

                bbox.bbox_reg[0] = *reg0;
                bbox.bbox_reg[1] = *reg1;
                bbox.bbox_reg[2] = *reg2;
                bbox.bbox_reg[3] = *reg3;

                object_box_node *node = object_box_node_create(&bbox);
                linked_list_insert_sort(objects_list, (linked_node *) node);
            }

            prob_data++;
            reg0++;
            reg1++;
            reg2++;
            reg3++;
        }
    }
}

void rnet_parse_object(object_box_t *orign_object_box, tensor_t *prob, tensor_t *box, float threshold) {
    float *score = (float *) prob->data;
    float *bbox = (float *) box->data;
    orign_object_box->score = (float) score[1];
    if (orign_object_box->score > threshold) {
        for (int c = 0; c < 4; c++) {
            orign_object_box->bbox_reg[c] = (float) bbox[c];
        }
    }
}

int mtcnn_detector_create(mtcnn_detector_t *detector)
{
    option_t opt = option_get_instance();
    int ret = network_setup_run_operations(detector->rnet, &opt);
    if(ret != CNET_STATUS_SUCCESS){
        printf("load r-net failed!\n");
        return ret;
    }

    ret = network_setup_run_operations(detector->pnet, &opt);
    if(ret != CNET_STATUS_SUCCESS){
        printf("load p-net failed!\n");
        network_teardown_run_operations(detector->rnet, &opt);
        return ret;
    }

    ret = network_setup_run_operations(detector->onet, &opt);
    if(ret != CNET_STATUS_SUCCESS){
        printf("load o-net failed!\n");
        network_teardown_run_operations(detector->onet, &opt);
        return ret;
    }

    return CNET_STATUS_SUCCESS;
}

void mtcnn_detector_release(mtcnn_detector_t *detector)
{
    option_t opt = option_get_instance();
    if(detector->rnet != NULL){
        network_teardown_run_operations(detector->rnet, &opt);
    }

    if(detector->pnet != NULL){
        network_teardown_run_operations(detector->pnet, &opt);
    }

    if(detector->onet != NULL){
        network_teardown_run_operations(detector->onet, &opt);
    }
}

static void save_tensor_to_image(tensor_t *image, char *file_name, image_file_type_t file_type) {
    image_t to_save_image;
    image_create(&to_save_image, image->d0, image->d1, image->d2, PIXEL_BGR_DATA);

    tensor_to_pixels(image, to_save_image.buf, PIXEL_BGR);
    save_image(to_save_image, file_type, file_name);

    image_free(&to_save_image);
}


linked_list run_pnet(network_t *pnet, pnet_param_t *pnet_param, tensor_t *img){
    linked_list results = create_linked_list((linked_node_compare)object_box_node_compare);
    float scale_w = 160.0 / img->d0;

    option_t opt = option_get_instance();
    tensor_t in = tensor_resize_bilinear(img, 160, 120, &opt);

    session_t session = session_create(pnet);
    session.option.light_mode = 1;
    session_set_input(&session, pnet_param->input_blob_index, in);
    tensor_release(&in);

    int ret = session_run(&session);
    if(CNET_STATUS_SUCCESS != ret){
        printf("get prob_blob %d failed\n", pnet_param->prob_blob_index);
        session_release(&session);
        return results;
    }

    tensor_t score = session_get_output(&session, pnet_param->prob_blob_index);
    if (0 == tensor_total(&score)){
        printf("get prob_blob %d failed\n", pnet_param->prob_blob_index);
        session_release(&session);
        return results;
    }

    tensor_t location = session_get_output(&session, pnet_param->location_blob_index);
    if (0 == tensor_total(&location)) {
        printf("get location blob %d failed\n", pnet_param->location_blob_index);
        tensor_release(&score);
        session_release(&session);
        return results;
    }

    pnet_parse_object(pnet_param, &score, &location, scale_w, &results);
    object_boxes_nms(&results, pnet_param->nms, object_box_iou_union);

    tensor_release(&score);
    tensor_release(&location);
    session_release(&session);

    return results;
}

linked_list run_rnet(network_t *rnet, rnet_param_t *rnet_param, tensor_t *img, linked_list *objects_list){
    int img_w = img->d0;
    int img_h = img->d1;

    linked_list results = create_linked_list((linked_node_compare)object_box_node_compare);
    int count = 0;
    for (linked_node *current = objects_list->head; NULL != current; )
    {
        object_box_t *bx = &((object_box_node*)current)->box;
        tensor_t img_t = tensor_cut_border(
                img,
                bx->y_min, img_h - bx->y_max,
                bx->x_min, img_w - bx->x_max,
                NULL);
        option_t opt = option_get_instance();
        tensor_t in = tensor_resize_bilinear(&img_t, rnet_param->input_w, rnet_param->input_h, &opt);
        const float norm_vals[3] = {1/0.0078125f, 1/0.0078125f, 1/0.0078125f};
        tensor_substract_mean_normalize(&in, 0, norm_vals);
        char buffer[156] = {0};
        sprintf(buffer, "rsize-image-%d", count ++);
        save_tensor_to_image(&in, buffer, IMAGE_FILE_TYPE_PNG);
        tensor_release(&img_t);

        session_t session = session_create(rnet);
        session_set_input(&session, rnet_param->input_blob_index, in);
        tensor_release(&in);

        int ret = session_run(&session);
        if(CNET_STATUS_SUCCESS != ret){
            printf("get prob_blob %d failed\n", rnet_param->prob_blob_index);
            session_release(&session);
            return results;
        }

        tensor_t score = session_get_output(&session, rnet_param->prob_blob_index);
        if (0 == tensor_total(&score)){
            printf("get prob_blob %d failed\n", rnet_param->prob_blob_index);
            session_release(&session);
            return results;
        }

        tensor_t location = session_get_output(&session, rnet_param->location_blob_index);
        if (0 == tensor_total(&location)) {
            printf("get location blob %d failed\n", rnet_param->location_blob_index);
            tensor_release(&score);
            session_release(&session);
            return results;
        }

        rnet_parse_object(bx, &score, &location, rnet_param->threshold_prob);
        tensor_release(&score);
        tensor_release(&location);
        session_release(&session);

        linked_node *tojust = current;
        current = current->next;
        linked_list_delete_node(objects_list, tojust);
        if(bx->score > rnet_param->threshold_prob){
            linked_list_insert_sort(&results, tojust);
        }else{
            fast_free(tojust);
        }
    }

    return results;
}

linked_list run_onet(network_t *onet,
                     onet_param_t *onet_param,
                     tensor_t *img,
                     onet_detect_parse_object onet_parse,
                     linked_list *objects_list){
    int img_w = img->d0;
    int img_h = img->d1;

    linked_list results = create_linked_list((linked_node_compare)object_box_node_compare);
    for (linked_node *current = objects_list->head; NULL != current; )
    {
        object_box_t *bx = &((object_box_node*)current)->box;
        tensor_t img_t = tensor_cut_border(img,bx->y_min, img_h - bx->y_max, bx->x_min, img_w - bx->x_max, NULL);

        tensor_t in = tensor_resize_bilinear(&img_t, onet_param->input_w, onet_param->input_h, NULL);
        tensor_release(&img_t);

        session_t session = session_create(onet);
        session_set_input(&session, onet_param->input_blob_index, in);
        tensor_release(&in);

        int ret = session_run(&session);
        if(CNET_STATUS_SUCCESS != ret){
            printf("get prob_blob %d failed\n", onet_param->prob_blob_index);
            session_release(&session);
            return results;
        }

        tensor_t score = session_get_output(&session, onet_param->prob_blob_index);
        if (0 == tensor_total(&score)){
            printf("get prob_blob %d failed\n", onet_param->prob_blob_index);
            session_release(&session);
            return results;
        }

        tensor_t location = session_get_output(&session, onet_param->location_blob_index);
        if (0 == tensor_total(&location)) {
            printf("get location blob %d failed\n", onet_param->location_blob_index);
            tensor_release(&score);
            session_release(&session);
            return results;
        }

        tensor_t landmark = session_get_output(&session, onet_param->landmark_blob_index);
        if (0 == tensor_total(&landmark)) {
            printf("get landmark blob %d failed\n", onet_param->landmark_blob_index);
            tensor_release(&score);
            tensor_release(&location);
            session_release(&session);
            return results;
        }

        onet_parse_param_t parse_param ={
                &score,
                &location,
                &landmark,
                onet_param->threshold_prob
        };
        onet_parse(bx, &parse_param, &results);

        tensor_release((&in));
        session_release(&session);

        linked_list_delete_node(objects_list, current);
        fast_free(current);
        current = objects_list->head;
    }

    return results;
}
