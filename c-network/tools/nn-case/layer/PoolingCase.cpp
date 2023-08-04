/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/pooling.h>
#include "PoolingCase.h"

PoolingCase::PoolingCase(CNetCase *network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool PoolingCase::ignore() {
    return false;
}

bool PoolingCase::quantize_weights() {
    return true;
}

static std::vector<int> get_pooling_runtime_padding(ncnn::Pooling *pooling, int w, int h) {

    int pad_left= pooling->pad_left;
    int pad_right = pooling->pad_right;
    int pad_top = pooling->pad_top;
    int pad_bottom = pooling->pad_bottom;
    if (pooling->pad_mode == 0) // full padding
    {
        int wtail = (w + pooling->pad_left + pooling->pad_right - pooling->kernel_w) % pooling->stride_w;
        int htail = (h + pooling->pad_top + pooling->pad_bottom - pooling->kernel_h) % pooling->stride_h;

        int wtailpad = 0;
        int htailpad = 0;

        if (wtail != 0)
            wtailpad = pooling->stride_w - wtail;
        if (htail != 0)
            htailpad = pooling->stride_h - htail;

        pad_bottom = pooling->pad_bottom + htailpad;
        pad_right = pooling->pad_right + wtailpad;

    } else if (pooling->pad_mode == 2)
    {
        // tensorflow padding=SAME or onnx padding=SAME_UPPER
        pad_right = pad_bottom = pad_left = pad_top = 0;
        int wpad = pooling->kernel_w + (w - 1) / pooling->stride_w * pooling->stride_w - w;
        int hpad = pooling->kernel_h + (h - 1) / pooling->stride_h * pooling->stride_h - h;
        if (wpad > 0 || hpad > 0) {
            pad_top =  hpad / 2;
            pad_bottom = hpad - hpad / 2;
            pad_left = wpad / 2;
            pad_right = wpad - wpad / 2;
        }

    } else if (pooling->pad_mode == 3)
    {
        // onnx padding=SAME_LOWER
        pad_right = pad_bottom = pad_left = pad_top = 0;
        int wpad = pooling->kernel_w + (w - 1) / pooling->stride_w * pooling->stride_w - w;
        int hpad = pooling->kernel_h + (h - 1) / pooling->stride_h * pooling->stride_h - h;
        if (wpad > 0 || hpad > 0) {
            pad_top = hpad - hpad / 2;
            pad_bottom = hpad / 2;
            pad_left = wpad - wpad / 2;
            pad_right = wpad / 2;
        }
    }

    return {pad_left, pad_right, pad_top, pad_bottom};
}

bool PoolingCase::get_layer_define(std::string &layer_define) {
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    auto *pool = dynamic_cast<ncnn::Pooling *>(layer);
    int inw = (int)pool->bottom_shapes[0].w;
    int inh = (int)pool->bottom_shapes[0].h;

    int16_t pad_value = 0;
    if (pool->pooling_type == ncnn::Pooling::PoolMethod_MAX)
    {
        pad_value = INT8_MIN;
    }

    std::vector<int> padding = get_pooling_runtime_padding(pool, inw, inh);

    char buffer[1024] = {0};
    sprintf(buffer,
            "DEFINE_POOLING_LAYER(%s,"
            " %d, %d, %d, %d, %d,"
            " %d,"
            " %d, %d, %d, %d, %hd,"
            " %d);\n",
            pool->name.c_str(),
            pool->pooling_type, pool->kernel_w, pool->kernel_h, pool->stride_w, pool->stride_h,
            pool->global_pooling,
            padding[0], padding[1], padding[2], padding[3], pad_value,
            pool->avgpool_count_include_pad);

    layer_define = buffer;

    return true;
}

bool PoolingCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(pooling, " + layer->name + ")";
    return true;
}

IMPL_CASE(Pooling)