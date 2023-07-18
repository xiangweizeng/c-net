/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/interp.h>
#include "InterpCase.h"

InterpCase::InterpCase(CNetCase *network, ncnn::Layer *layer)
: CNetLayerCase(network, layer) {}

bool InterpCase::ignore() {
    return false;
}

bool InterpCase::quantize_weights() {
    return true;
}

bool InterpCase::get_layer_define(std::string &layer_define) {
    ncnn::Interp *ip = dynamic_cast<ncnn::Interp *>(layer);

    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_UP_SAMPLE_LAYER(%s, %d, %f, %f);\n",
            ip->name.c_str(), ip->resize_type, ip->width_scale, ip->height_scale);

    layer_define = buffer;
    return true;
}

bool InterpCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(upsample, " + layer->name + ")";
    return true;
}

IMPL_CASE(Interp)