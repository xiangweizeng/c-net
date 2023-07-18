/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "SigmoidCase.h"

SigmoidCase::SigmoidCase(CNetCase *network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool SigmoidCase::ignore() {
    return false;
}

bool SigmoidCase::quantize_weights() {
    return true;
}

bool SigmoidCase::get_layer_define(std::string &layer_define) {
    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_SIGMOID_LAYER(%s);\n", layer->name.c_str());
    layer_define = buffer;
    return true;
}

bool SigmoidCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(sigmoid, " + layer->name + ")";
    return true;
}

IMPL_CASE(Sigmoid)