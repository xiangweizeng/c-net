/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_LPC_RECOGNIZER_H
#define CNET_LPC_RECOGNIZER_H

#include "network.h"
#include "detection/affine_matrix.h"
#include "lpr/plate_info.h"

typedef struct lpc_recognizer_t {
    network_t *network;
    int input_blob_index;
    int color_blob_index;
} lpc_recognizer_t;

/**
 * Create lpc_recognizer
 * @param lpc
 * @param network
 * @param input_index
 * @param color_index
 * @return
 */
status_t lpc_recognizer_create(
        lpc_recognizer_t *lpc,
        network_t *network,
        int input_index,
        int color_index);

/**
 * Release lpc_recognizer
 * @param lpc
 */
void lpc_recognizer_release(
        lpc_recognizer_t *lpc);

/**
 * lpc_recognizer predict color
 * @param lpc
 * @param plate
 * @return
 */
plate_color_t lpc_recognizer_predict_color(
        lpc_recognizer_t *lpc,
        tensor_t *plate);

/**
 * get color name
 * @param color
 * @return
 */
const char *lpc_recognizer_get_color_name(
        plate_color_t color);

#endif //CNET_LPC_RECOGNIZER_H
