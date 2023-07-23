/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/permute.h>
#include <layer/reshape.h>
#include "PermuteCase.h"

PermuteCase::PermuteCase(CNetCase* network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool PermuteCase::ignore() {
    const auto* permute = dynamic_cast<const ncnn::Permute*>(layer);

    if(blobs[layer->tops[0]].consumer <= 0){
        return true;
    }

    if(blobs[layer->tops[0]].consumer >= 0 && layers[blobs[layer->tops[0]].consumer]->type == "Reshape"){
        const auto* reshape = dynamic_cast<const ncnn::Reshape*>(layers[blobs[layer->tops[0]].consumer]);
        if(permute->order_type == 3 && reshape->ndim <= 2){
            return true;
        }
    }
    return false;
}

bool PermuteCase::quantize_weights() {
    return true;
}

bool PermuteCase::get_layer_define(std::string &layer_define) {
    // 0 = w h
    // 1 = h w

    // order_type
    // 0 = w h c
    // 1 = h w c
    // 2 = w c h
    // 3 = c w h
    // 4 = h c w
    // 5 = c h w

    const auto* permute = dynamic_cast<const ncnn::Permute*>(layer);
    int perm = permute->order_type + (permute->bottom_shapes[0].dims == 2 ? 0 : 2);

    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_PERMUTE_LAYER(%s, %d);\n", permute->name.c_str(), perm);
    layer_define = buffer;

    return true;
}

bool PermuteCase::get_layer_reference(std::string &reference) {
    reference =  "REFERENCE_LAYER(permute, " + layer->name + ")";
    return true;
}

IMPL_CASE(Permute)
