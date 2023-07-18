/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/slice.h>
#include "SliceCase.h"

SliceCase::SliceCase(CNetCase *network, ncnn::Layer *layer)
: CNetLayerCase(network, layer) {}

bool SliceCase::ignore() {
    return false;
}

bool SliceCase::quantize_weights() {
    ncnn::Slice *sl = dynamic_cast<ncnn::Slice *>(layer);
    std::string param_var = "layer_" + layer->name + "_slice_slices";
    quantize_data_weights[param_var] = sl->slices.clone();
    return true;
}

bool SliceCase::get_layer_define(std::string &layer_define) {

    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    ncnn::Slice *sl = dynamic_cast<ncnn::Slice *>(layer);
    float input_scale = case_blobs[input].scale;
    float output_scale = case_blobs[output].scale;

    fixed_mul_t requantize = get_fixed_mul(output_scale / (input_scale));
    char buffer[1024] = {0};
    sprintf(buffer,
            "DEFINE_SLICE_LAYER(%s, %d, %d, %d);\n",
            sl->name.c_str(), sl->axis, requantize.round_mul, requantize.shift
    );

    layer_define = buffer;

    return true;
}

bool SliceCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(slice, " + layer->name + ")";
    return true;
}

IMPL_CASE(Slice)