/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/prelu.h>
#include "PReLUCase.h"

PReLUCase::PReLUCase(CNetCase *network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool PReLUCase::ignore() {
    return false;
}

bool PReLUCase::quantize_weights() {
    ncnn::PReLU *relu = dynamic_cast<ncnn::PReLU *>(layer);
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    float input_scale = case_blobs[input].scale;
    float output_scale = case_blobs[output].scale;

    ncnn::Mat slope_data = relu->slope_data.clone();
    for(int i = 0; i < relu->num_slope; i++){
        slope_data[i] =  slope_data[i] * output_scale / input_scale;
    }

    std::string param_var = "layer_" + relu->name + "_prelu_slope";
    quantize_data_weights[param_var] = slope_data;

    return true;
}

bool PReLUCase::get_layer_define(std::string &layer_define) {
    ncnn::PReLU *relu = dynamic_cast<ncnn::PReLU *>(layer);
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    float input_scale = case_blobs[input].scale;
    float output_scale = case_blobs[output].scale;
    fixed_mul_t requantize = get_fixed_mul(output_scale / input_scale);

    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_PRELU_LAYER(%s, %d, %d, %d);\n",
            relu->name.c_str(), relu->num_slope,
            requantize.round_mul, requantize.shift);

    layer_define = buffer;
    return true;
}

bool PReLUCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(prelu, " + layer->name + ")";
    return true;
}

IMPL_CASE(PReLU)