/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_LPR_RECOGNIZER_H
#define CNET_LPR_RECOGNIZER_H

#include "network.h"
#include "plate_info.h"

typedef struct lpr_recognizer_t{
    network_t *network;
    int input_blob_index;
    int plate_blob_index;
}lpr_recognizer_t;

/**
 * create lpr_recognizer
 * @param network
 * @param input_index
 * @param plate_index
 * @return
 */
status_t lpr_recognizer_create(
        lpr_recognizer_t *lpr,
        network_t *network,
        int input_index,
        int plate_index);

/**
 * reloease lpr_recognizer
 * @param lpr
 */
void lpr_recognizer_release(lpr_recognizer_t *lpr);

/**
 * lpr_recognizer predict number
 * @param lpr
 * @param plate
 * @param plate_info
 * @return
 */
void lpr_recognizer_predict_number(
        lpr_recognizer_t *lpr,
        tensor_t *plate,
        plate_info_t *plate_info);

#endif //CNET_LPR_RECOGNIZER_H
