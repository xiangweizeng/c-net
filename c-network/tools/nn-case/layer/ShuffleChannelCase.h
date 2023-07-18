/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_SHUFFLE_CHANNEL_CASE_H
#define CNET_SHUFFLE_CHANNEL_CASE_H

#include "../CNetLayerCase.h"

class ShuffleChannelCase : public CNetLayerCase{
public:
    ShuffleChannelCase(CNetCase *network, ncnn::Layer *layer);

    bool ignore() override;

    bool quantize_weights() override;

    bool get_layer_define(std::string &layer_define) override;

    bool get_layer_reference(std::string &reference) override;
};


#endif //CNET_SHUFFLE_CHANNEL_CASE_H
