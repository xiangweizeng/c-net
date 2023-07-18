/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/unaryop.h>
#include "UnaryOpCase.h"

UnaryOpCase::UnaryOpCase(CNetCase *network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool UnaryOpCase::ignore() {
    return false;
}

bool UnaryOpCase::quantize_weights() {
    return true;
}

bool UnaryOpCase::get_layer_define(std::string &layer_define) {
    ncnn::UnaryOp *op = dynamic_cast<ncnn::UnaryOp *>(layer);

    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_UNARY_LAYER(%s, %d);\n", op->name.c_str(), op->op_type);
    layer_define = buffer;
    return true;
}

bool UnaryOpCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(unary, " + layer->name + ")";
    return true;
}

IMPL_CASE(UnaryOp)