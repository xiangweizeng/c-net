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
    auto *relu = dynamic_cast<ncnn::PReLU *>(layer);
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    float input_scale = case_blobs[input].scale;
    float output_scale = case_blobs[output].scale;

    ncnn::Mat slope_data = relu->slope_data.clone();
    for(int i = 0; i < relu->num_slope; i++){
        slope_data[i] =  slope_data[i] * output_scale / input_scale;
    }

    QuantizeMat quantize_slope_data(slope_data, float_data_type);
    std::string param_var = "layer_" + relu->name + "_prelu_slope";
    quantize_data_weights[param_var] = quantize_slope_data;

    return true;
}

bool PReLUCase::get_layer_define(std::string &layer_define) {
    auto *relu = dynamic_cast<ncnn::PReLU *>(layer);
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    float input_scale = case_blobs[input].scale;
    float output_scale = case_blobs[output].scale;
    float requantize = output_scale / input_scale;

    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_PRELU_LAYER(%s, %d, %f);\n", relu->name.c_str(), relu->num_slope, requantize);

    layer_define = buffer;
    return true;
}

bool PReLUCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(prelu, " + layer->name + ")";
    return true;
}

IMPL_CASE(PReLU)