/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/memorydata.h>
#include "MemoryDataCase.h"

MemoryDataCase::MemoryDataCase(CNetCase *network, ncnn::Layer *layer)
: CNetLayerCase(network, layer) {}

bool MemoryDataCase::ignore() {
    return false;
}

bool MemoryDataCase::quantize_weights() {
    std::string output = get_blob_output_name(0);

    ncnn::MemoryData * md = (ncnn::MemoryData*)layer;
    float weight_scale = quantize->get_scaled(md->data);
    std::string param_var = "layer_" + md->name + "_shuffle_channel_data";
    quantize_data_weights[param_var] = quantize->do_quantize(md->data, weight_scale);
    return true;
}

bool MemoryDataCase::get_layer_define(std::string &layer_define) {
    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_MEMORY_DATA_LAYER(%s);\n", layer->name.c_str());
    layer_define = buffer;
    return true;
}

bool MemoryDataCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(memory_data, " + layer->name + ")";
    return true;
}

IMPL_CASE(MemoryData)
