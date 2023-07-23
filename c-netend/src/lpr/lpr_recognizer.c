/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "lpr/lpr_recognizer.h"
#include "tensor_pixel.h"
#include "tensor_operation.h"

static char* plate_labels[] = {
        " ",
        "#",
        "京",
        "津",
        "冀",
        "晋",
        "蒙",
        "辽",
        "吉",
        "黑",
        "沪",
        "苏",
        "浙",
        "皖",
        "闽",
        "赣",
        "鲁",
        "豫",
        "鄂",
        "湘",
        "粤",
        "桂",
        "琼",
        "渝",
        "川",
        "贵",
        "云",
        "藏",
        "陕",
        "甘",
        "青",
        "宁",
        "新",
        "使",
        "0",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "J",
        "K",
        "L",
        "M",
        "N",
        "O",
        "P",
        "Q",
        "R",
        "S",
        "T",
        "U",
        "V",
        "W",
        "X",
        "Y",
        "Z",
        "警",
        "学",
        "挂",
        "港",
        "澳",
        "领",
        "民",
        "航",
        "应",
        "急"
};


status_t lpr_recognizer_create(lpr_recognizer_t *lpr, network_t *network, int input_index, int plate_index)
{
    lpr->network = network;
    lpr->input_blob_index = input_index;
    lpr->plate_blob_index = plate_index;

    if(CNET_STATUS_FAILED  == network_setup_run_operations(lpr->network, NULL)){
        printf("lpr load bin failed\n");
        return CNET_STATUS_FAILED;
    }

    return CNET_STATUS_SUCCESS;
}

void lpr_recognizer_release(lpr_recognizer_t *lpr)
{
    if(lpr->network){
        network_teardown_run_operations(lpr->network, NULL);
        lpr->network = NULL;
    }
}

vector_int lpr_recognizer_decode(tensor_t *output)
{
    int prev_class_idx = -1;
    vector_int codes = VECTOR_CREATE(int, 0);

    float *current_code = output->data;
    for (int t = 0; t < output->d1; t++)
    {

        float max_prob = *current_code;
        int max_class_idx = 0;

        current_code++;
        for (int c = 1; c < output->d0; c++, current_code++)
        {
            if (*current_code > max_prob)
            {
                max_prob = *current_code;
                max_class_idx = c;
            }
        }

        if (max_class_idx != 0 && max_class_idx != prev_class_idx)
        {
            codes.push_element(&codes, &max_class_idx);
        }
        prev_class_idx = max_class_idx;
    }

    return  codes;
}

void lpr_recognizer_decode_number(vector_int *codes, plate_info_t *plate_info)
{
    memset(plate_info->plate_no, 0 , sizeof(plate_info->plate_no));
    for(int i = 0; i < codes->current_size; i++){
        strcat(plate_info->plate_no, plate_labels[codes->data[i]]);
    }
}

void lpr_recognizer_predict_number(lpr_recognizer_t *lpr, tensor_t *plate, plate_info_t *plate_info)
{
    const float mean_vals[3] = {116.407, 133.722, 124.187};
    tensor_substract_mean_normalize(plate, mean_vals, NULL);

    session_t session = session_create(lpr->network);
    session_set_input(&session, lpr->input_blob_index, *plate);

    if(CNET_STATUS_SUCCESS != session_run(&session)){
        printf("session extract plate_blob failed\n");
        session_release(&session);
        return;
    }

    tensor_t plate_blob = session_get_output(&session, lpr->plate_blob_index);
    if(0 == tensor_total(&plate_blob)){
        printf("session extract plate_blob failed\n");
        session_release(&session);
        return;
    }

    vector_int codes = lpr_recognizer_decode(&plate_blob);
    lpr_recognizer_decode_number(&codes, plate_info);

    codes.free_data(&codes);
    tensor_release(&plate_blob);
    session_release(&session);
}