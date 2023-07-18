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

    ncnn::BatchNorm * bn = (ncnn::BatchNorm*)layer;

    float output_scale = case_blobs[output].scale;
    float weight_scale = quantize->get_scaled(bn->b_data);
    std::string param_var = "layer_" + bn->name + "_batch_norm_scale";
    quantize_data_weights[param_var] = quantize->do_quantize(bn->b_data, weight_scale);

    param_var = "layer_" + bn->name + "_batch_norm_offset";
    quantize_data_weights[param_var] = quantize->do_quantize(bn->a_data, output_scale);

    return true;
}

bool BatchNormCase::get_layer_define(std::string &layer_define) {
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    ncnn::BatchNorm * bn = (ncnn::BatchNorm*)layer;
    float input_scale = case_blobs[input].scale;
    float output_scale = case_blobs[output].scale;
    float weight_scale = quantize->get_scaled(bn->b_data);
    fixed_mul_t requantize = get_fixed_mul(output_scale / (input_scale * weight_scale));

    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_BATCH_NORM_LAYER(%s, %d,%d);\n",
            bn->name.c_str(), requantize.round_mul, requantize.shift);
    layer_define = buffer;

    return false;
}

bool BatchNormCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(batch_norm, " + layer->name + ")";
    return true;
}

IMPL_CASE(BatchNorm)