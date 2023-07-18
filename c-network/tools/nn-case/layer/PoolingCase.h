/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_POOLING_CASE_H
#define CNET_POOLING_CASE_H

#include "../CNetLayerCase.h"

class PoolingCase : public CNetLayerCase{
public:
    PoolingCase(CNetCase *network, ncnn::Layer *layer);

    bool ignore() override;

    bool quantize_weights() override;

    bool get_layer_define(std::string &layer_define) override;

    bool get_layer_reference(std::string &reference) override;
};


#endif //CNET_POOLING_CASE_H
