/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/batchnorm.h>
#include "BatchNormCase.h"

BatchNormCase::BatchNormCase(CNetCase* network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool BatchNormCase::ignore() {
    return false;
}

bool BatchNormCase::quantize_weights() {

    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    auto * bn = (ncnn::BatchNorm*)layer;
    float input_scale = case_blobs[input].scale;
    float output_scale = case_blobs[output].scale;

    ncnn::Mat scaled_data = bn->b_data.clone();

    for(int i = 0; i < scaled_data.total(); i++){
        scaled_data[i] = (scaled_data[i] * output_scale) / input_scale;
    }
    QuantizeMat scaled_quantize_data(scaled_data, float_data_type);
    std::string param_var = "layer_" + bn->name + "_batch_norm_scale";
    quantize_data_weights[param_var] = scaled_quantize_data;

    param_var = "layer_" + bn->name + "_batch_norm_offset";
    ncnn::Mat output_quantize_data;
    output_quantize_data.create(1, 4u, nullptr);
    output_quantize_data[0] = output_scale;
    quantize_data_weights[param_var] = QuantizeMat(
            quantize->do_quantize_s32(bn->a_data, output_quantize_data),
            data_type);

    return true;
}

bool BatchNormCase::get_layer_define(std::string &layer_define) {
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    auto * bn = (ncnn::BatchNorm*)layer;
    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_BATCH_NORM_LAYER(%s);\n", bn->name.c_str());
    layer_define = buffer;

    return false;
}

bool BatchNormCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(batch_norm, " + layer->name + ")";
    return true;
}

IMPL_CASE(BatchNorm)