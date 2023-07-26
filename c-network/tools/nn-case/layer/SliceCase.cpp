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
    auto *sl = dynamic_cast<ncnn::Slice *>(layer);

    std::string param_var = "layer_" + layer->name + "_slice_slices";
    QuantizeMat quantize_slices(sl->slices, int32_data_type);
    quantize_data_weights[param_var] = quantize_slices;

    return true;
}

bool SliceCase::get_layer_define(std::string &layer_define) {

    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    auto *sl = dynamic_cast<ncnn::Slice *>(layer);
    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_SLICE_LAYER(%s, %d);\n", sl->name.c_str(), sl->axis);

    layer_define = buffer;

    return true;
}

bool SliceCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(slice, " + layer->name + ")";
    return true;
}

IMPL_CASE(Slice)