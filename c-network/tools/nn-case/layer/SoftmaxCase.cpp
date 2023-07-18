/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/softmax.h>
#include "SoftmaxCase.h"

SoftmaxCase::SoftmaxCase(CNetCase* network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool SoftmaxCase::ignore() {
    return false;
}

bool SoftmaxCase::quantize_weights() {
    return true;
}

bool SoftmaxCase::get_layer_define(std::string &layer_define) {
    const auto* softmax = dynamic_cast<const ncnn::Softmax*>(layer);
    int axis = softmax->axis + 4 - softmax->bottom_shapes[0].dims;

    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_SOFTMAX_LAYER(%s,%d);\n", softmax->name.c_str(), axis);
    layer_define = buffer;

    return true;
}

bool SoftmaxCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(softmax, " + layer->name + ")";;
    return false;
}

IMPL_CASE(Softmax)