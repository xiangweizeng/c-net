/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "lpr/lpc_recognizer.h"
#include "tensor_pixel.h"
#include "tensor_operation.h"

status_t lpc_recognizer_create(lpc_recognizer_t *lpc, network_t *network, int input_index, int color_index)
{
    lpc->input_blob_index = input_index;
    lpc->color_blob_index = color_index;
    lpc->network = network;

    if(CNET_STATUS_SUCCESS  != network_setup_run_operations(lpc->network, NULL)){
        printf("lpc load failed\n");
        return CNET_STATUS_FAILED;
    }

    return CNET_STATUS_SUCCESS;
}

void lpc_recognizer_release(lpc_recognizer_t *lpc)
{
    if(lpc->network){
        network_teardown_run_operations(lpc->network, NULL);
        lpc->network = NULL;
    }
}

plate_color_t  lpc_recognizer_predict_color(lpc_recognizer_t *lpc, tensor_t *plate)
{
    const float mean_vals[3] = {89.9372, 81.1989, 73.6352};
    tensor_substract_mean_normalize(plate, mean_vals, NULL);

    session_t session = session_create(lpc->network);
    session_set_input(&session, lpc->input_blob_index, *plate);

    if(CNET_STATUS_SUCCESS != session_run(&session)){
        printf("session extract plate_blob failed\n");
        session_release(&session);
        return PLATE_COLOR_UNKNOWN;
    }

    tensor_t probs_blob = session_get_output(&session, lpc->color_blob_index);
    if(0 == tensor_total(&probs_blob)){
        printf("session extract probs_blob failed\n");
        session_release(&session);
        return PLATE_COLOR_UNKNOWN;
    }

    float *probs = (float*)probs_blob.data;
    float max = probs[0];
    int clsId = 0;
    for (int i = 1; i < 5; i++)
    {
        if (probs[i] > max)
        {
            max = probs[i];
            clsId = i;
        }
    }

    tensor_release(&probs_blob);
    session_release(&session);
    return (plate_color_t)(clsId);
}

const char * lpc_recognizer_get_color_name(plate_color_t color)
{
    static const char* colors_name[] = {
                "白",
                "黄",
                "蓝",
                "黑",
                "绿",
    };

    if(color <= PLATE_COLOR_GREEN){
        return colors_name[color];
    }

    return "";
}