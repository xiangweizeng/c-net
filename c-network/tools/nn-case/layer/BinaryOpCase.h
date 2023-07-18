/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_BINARY_OP_CASE_H
#define CNET_BINARY_OP_CASE_H

#include "../CNetLayerCase.h"

class BinaryOpCase : public CNetLayerCase{
public:
    BinaryOpCase(CNetCase* network, ncnn::Layer *layer);

    bool ignore() override;

    bool quantize_weights() override;

    bool get_layer_define(std::string &layer_define) override;

    bool get_layer_reference(std::string &reference) override;
};


#endif //CNET_BINARY_OP_CASE_H
