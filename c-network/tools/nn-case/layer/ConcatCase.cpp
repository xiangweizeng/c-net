/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/concat.h>
#include "ConcatCase.h"

bool ConcatCase::ignore() {
    return false;
}

bool ConcatCase::quantize_weights() {
    return true;
}

bool ConcatCase::get_layer_define(std::string &layer_define) {

    const auto* concat = dynamic_cast<const ncnn::Concat*>(layer);
    std::string output = get_blob_output_name(0);
    float output_scale = case_blobs[output].scale;

    std::string input;
    for (size_t j = 0; j < concat->bottoms.size(); j++) {
        std::string input_blob = get_blob_input_name((int)j);
        float input_scale = case_blobs[input_blob].scale;
        fixed_mul_t fixed = get_fixed_mul(output_scale / input_scale);

        char concat_blob[256] = {0};
        sprintf(concat_blob, "%d, %d", fixed.round_mul, fixed.shift);

        if (j == 0) {
            input = concat_blob;
        }
        else {
            input += ", " + std::string(concat_blob);
        }
    }

    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_CONCAT_LAYER(%s, %d, %d, %s);\n",
            concat->name.c_str(),
            concat->axis + 4 - concat->top_shapes[0].dims,
            (int)concat->bottoms.size(),
            input.c_str());

    layer_define = buffer;

    return true;
}

bool ConcatCase::get_layer_reference(std::string &reference) {
    reference ="REFERENCE_LAYER(concat, " + layer->name + ")";
    return true;
}

ConcatCase::ConcatCase(CNetCase* network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {
}

IMPL_CASE(Concat)