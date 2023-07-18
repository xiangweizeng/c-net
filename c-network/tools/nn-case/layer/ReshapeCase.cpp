/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "ReshapeCase.h"

ReshapeCase::ReshapeCase(CNetCase* network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool ReshapeCase::ignore() {
    return false;
}

bool ReshapeCase::quantize_weights() {
    return true;
}

bool ReshapeCase::get_layer_define(std::string &layer_define) {
    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_RESHAPE_LAYER(%s);\n", layer->name.c_str());
    layer_define = buffer;
    return true;
}

bool ReshapeCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(reshape, " + layer->name + ")";
    return true;
}

static PCNetLayerCase CaseCreator(CNetCase* network, ncnn::Layer *layer){
    return std::make_shared<ReshapeCase>(network, layer);
}

REGISTER_CASE(Reshape, CaseCreator)
REGISTER_CASE(Squeeze, CaseCreator)
REGISTER_CASE(Flatten, CaseCreator)
REGISTER_CASE(Expanddims, CaseCreator)
