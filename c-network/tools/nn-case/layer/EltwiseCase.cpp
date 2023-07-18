/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/eltwise.h>
#include <layer/binaryop.h>
#include "EltwiseCase.h"

EltwiseCase::EltwiseCase(CNetCase *network, ncnn::Layer *layer)
: CNetLayerCase(network, layer) {}

bool EltwiseCase::ignore() {
    return false;
}

bool EltwiseCase::quantize_weights() {
    return true;
}

bool EltwiseCase::get_layer_define(std::string &layer_define) {
    ncnn::Eltwise *op = dynamic_cast<ncnn::Eltwise *>(layer);
    if(op->bottoms.size() > 2){
        return false;
    }

    if(!op->coeffs.empty()){
        return false;
    }

    int op_type;
    switch (op->op_type) {
        case  ncnn::Eltwise::Operation_PROD:{
            op_type = ncnn::BinaryOp::Operation_MUL;
            break;
        }
        case  ncnn::Eltwise::Operation_SUM:{
            op_type = ncnn::BinaryOp::Operation_ADD;
            break;
        }
        case  ncnn::Eltwise::Operation_MAX:{
            op_type = ncnn::BinaryOp::Operation_MAX;
            break;
        }
        default:{
            return false;
        }
    }

    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_BINARY_LAYER(%s, %d, %d, %f);\n", op->name.c_str(), op_type, 0, 0.f);
    layer_define = buffer;
    return true;
}

bool EltwiseCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(binary, " + layer->name + ")";
    return true;
}

IMPL_CASE(Eltwise)
