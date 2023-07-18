/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/




#include "face/arc_face.h"
#include "network.h"

/**
 * Create arc face
 * @param arc_face
 * @return
 */
int arc_face_create(arc_face_t* arc_face)
{
    if(NULL == arc_face->net){
        return CNET_STATUS_FAILED;
    }

    option_t opt = option_get_instance();
    int ret = network_setup_run_operations(arc_face->net, &opt);
    if(ret != CNET_STATUS_SUCCESS){
        printf("load arcface bin failed\n");
        return ret;
    }

    return  0;
}

/**
 * release arcface
 * @param arc_face
 */
void arc_face_release(arc_face_t *arc_face)
{
    if(arc_face->net != NULL){
        option_t opt = option_get_instance();
        network_teardown_run_operations(arc_face->net, &opt);
    }
}

/**
 * Get vector from face and origin image
 * @param arc_face
 * @param image
 * @param face_info
 * @return
 */
vector_float arc_face_get_face_feature(
        arc_face_t *arc_face,
        tensor_t *image){

    VECTOR_DECLARE(float, feature) = VECTOR_CREATE(float, 0);

    session_t session = session_create(arc_face->net);
    session_set_input(&session, arc_face->input_blob_index, *image);
    session_run(&session);

    tensor_t feature_tensor = session_get_output(&session, arc_face->vector_blob_index);
    if (0 == tensor_total(&feature_tensor)) {
        printf("Get feature %zu failed\n", arc_face->vector_blob_index);
        session_release(&session);
        return feature;
    }

    feature.free_data(&feature);
    int feature_dim = tensor_total(&feature_tensor);
    feature = VECTOR_CREATE(float, feature_dim);
    feature.current_size = feature_dim;

    memcpy(feature.data, feature_tensor.data, sizeof(float) * feature_dim);
    tensor_release(&feature_tensor);
    face_feature_normalize(feature.data, feature_dim);

    session_release(&session);
    return feature;
}