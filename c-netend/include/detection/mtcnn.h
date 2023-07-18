/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_MTCNN_H
#define CNET_MTCNN_H

#include "network.h"
#include "affine_matrix.h"
#include "object_box.h"

typedef struct pnet_param_t{
    float stride;
    float cell_width;
    float cell_height;
    float threshold_prob;
    int minl_threshold;
    float nms;
    float factor;
    int input_blob_index;
    int prob_blob_index;
    int location_blob_index;
    int min_size;
}pnet_param_t;

/**
 * parse pnet ouput tensor to objects_list
 * @param param
 * @param prob
 * @param box
 * @param objects_list
 */
FUNCTION_IRAM void pnet_parse_object(
        pnet_param_t* param,
        tensor_t *prob,
        tensor_t *box,
        float scale,
        linked_list *objects_list);

typedef struct rnet_param_t{
    int input_w;
    int input_h;
    float threshold_prob;
    float nms;
    int input_blob_index;
    int prob_blob_index;
    int location_blob_index;
}rnet_param_t;


/**
 * parse rnet output tensor to orign object
 * @param orign_object_box
 * @param prob
 * @param box
 * @param threshold
 */
FUNCTION_IRAM void rnet_parse_object(
        object_box_t *orign_object_box,
        tensor_t *prob,
        tensor_t *box,
        float threshold);

typedef struct onet_param_t{
    int input_w;
    int input_h;
    float threshold_prob;
    float nms;
    int input_blob_index;
    int prob_blob_index;
    int location_blob_index;
    int landmark_blob_index;
}onet_param_t;

typedef struct onet_parse_param_t{
    tensor_t *prob;
    tensor_t *box;
    tensor_t *landmark;
    float threshold;
}onet_parse_param_t;

/**
 * onet object parse function
 */
typedef void (*onet_detect_parse_object)(object_box_t *, onet_parse_param_t * ,  linked_list *);

/**
 * Mtcnn detector
 */
typedef struct mtcnn_detector_t{
    network_t *pnet;
    network_t *rnet;
    network_t *onet;

    pnet_param_t pnet_param;
    rnet_param_t rnet_param;
    onet_param_t onet_param;
    float side_scale;
}mtcnn_detector_t;

/**
 *  Create default mtcnn detector, from config
 * @param detector  need to init detector
 * @return  if return 0 ,init successful, else failed
 */
int mtcnn_detector_create(mtcnn_detector_t *detector);

/**
 *  Free network
 * @param detector
 */
void mtcnn_detector_release(mtcnn_detector_t *detector);


/**
 * Run Pnet
 * @param pnet
 * @param pnet_param
 * @param img
 * @return
 */
linked_list run_pnet(
        network_t *pnet,
        pnet_param_t *pnet_param,
        tensor_t *img);

/**
 * Run Rnet
 * @param rnet
 * @param rnet_param
 * @param img
 * @param objects_list
 * @return
 */
linked_list run_rnet(
        network_t *rnet,
        rnet_param_t *rnet_param,
        tensor_t *img,
        linked_list *objects_list);
/**
 * Run Onet
 * @param onet
 * @param onet_param
 * @param img
 * @param onet_parse
 * @param objects_list
 * @return
 */
linked_list run_onet(
        network_t *onet,
        onet_param_t *onet_param,
        tensor_t *img,
        onet_detect_parse_object onet_parse,
        linked_list *objects_list);

#endif //CNET_MTCNN_H
