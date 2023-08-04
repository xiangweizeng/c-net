/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/padding.h>
#include "PaddingCase.h"

PaddingCase::PaddingCase(CNetCase *network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool PaddingCase::ignore() {
    return false;
}

bool PaddingCase::quantize_weights() {
    return true;
}

bool PaddingCase::get_layer_define(std::string &layer_define) {
    auto *pd = dynamic_cast<ncnn::Padding *>(layer);
    std::string output = get_blob_output_name(0);
    float output_scale = case_blobs[output].scale;

    int pad_value = float2int8(output_scale * pd->value);
    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_PADDING_LAYER(%s, %d, %d, %d, %d, %d, %d);\n", pd->name.c_str(),
            pd->type, pd->left, pd->right, pd->top, pd->bottom, pad_value);
    layer_define = buffer;

    return true;
}

bool PaddingCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(padding, " + layer->name + ")";
    return true;
}

IMPL_CASE(Padding)
