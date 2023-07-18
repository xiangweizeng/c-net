/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/binaryop.h>
#include "BinaryOpCase.h"

BinaryOpCase::BinaryOpCase(CNetCase* network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool BinaryOpCase::ignore() {
    return false;
}

bool BinaryOpCase::quantize_weights() {
    return true;
}

bool BinaryOpCase::get_layer_define(std::string &layer_define) {
    ncnn::BinaryOp *op = dynamic_cast<ncnn::BinaryOp *>(layer);

    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_BINARY_LAYER(%s, %d, %d, %f);\n",
            op->name.c_str(), op->op_type, op->with_scalar, op->b);
    layer_define = buffer;

    return true;
}

bool BinaryOpCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(binary, " + layer->name + ")";
    return true;
}

IMPL_CASE(BinaryOp)