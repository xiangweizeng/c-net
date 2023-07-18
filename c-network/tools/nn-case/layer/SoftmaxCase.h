/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_SOFT_MAX_CASE_H
#define CNET_SOFT_MAX_CASE_H

#include "../CNetLayerCase.h"

class SoftmaxCase : public CNetLayerCase{
public:
    SoftmaxCase(CNetCase* network, ncnn::Layer *layer);

    bool ignore() override;

    bool quantize_weights() override;

    bool get_layer_define(std::string &layer_define) override;

    bool get_layer_reference(std::string &reference) override;
};


#endif //CNET_SOFT_MAX_CASE_H
