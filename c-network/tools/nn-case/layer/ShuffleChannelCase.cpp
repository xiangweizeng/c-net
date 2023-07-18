/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/shufflechannel.h>
#include "ShuffleChannelCase.h"

ShuffleChannelCase::ShuffleChannelCase(CNetCase *network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool ShuffleChannelCase::ignore() {
    return false;
}

bool ShuffleChannelCase::quantize_weights() {
    return true;
}

bool ShuffleChannelCase::get_layer_define(std::string &layer_define) {
    char buffer[1024] = {0};

    ncnn::ShuffleChannel *sc = reinterpret_cast<ncnn::ShuffleChannel *>(layer);
    sprintf(buffer, "DEFINE_SHUFFLE_CHANNEL_LAYER(%s, %d);\n", sc->name.c_str(), sc->group);
    layer_define = buffer;
    return true;
}

bool ShuffleChannelCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(shuffle_channel, " + layer->name + ")";
    return true;
}

IMPL_CASE(ShuffleChannel)