/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/innerproduct.h>
#include "InnerProductCase.h"

InnerProductCase::InnerProductCase(CNetCase *network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool InnerProductCase::ignore() {
    return false;
}

bool InnerProductCase::quantize_weights() {

    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    auto *ip = dynamic_cast<ncnn::InnerProduct *>(layer);

    float output_scale = case_blobs[output].scale;
    float weight_scale = quantize->get_scaled(ip->weight_data);
    std::string param_var = "layer_" + ip->name + "_inner_product_filters";
    auto quantize_data = quantize->do_quantize(ip->weight_data, weight_scale);

    if(ip->bottom_shapes[0].dims == 3){
        int n = ip->num_output;
        int h = (int)ip->bottom_shapes[0].h;
        int w = (int)ip->bottom_shapes[0].w;
        int c = (int)ip->bottom_shapes[0].c;

        quantize_data = quantize->permute(quantize_data, n, h, w, c);
    }
    quantize_data_weights[param_var] = quantize_data;

    param_var = "layer_" + ip->name + "_inner_product_bias";
    if (ip->bias_term) {
        quantize_data_weights[param_var] = quantize->do_quantize(ip->bias_data, output_scale);
    } else{
        quantize_data_weights[param_var] = ncnn::Mat();
    }

    return true;
}

bool InnerProductCase::get_layer_define(std::string &layer_define) {
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    auto *ip = dynamic_cast<ncnn::InnerProduct *>(layer);
    int inw = (int)ip->bottom_shapes[0].w;
    int inh = (int)ip->bottom_shapes[0].h;
    int inc = (int)ip->bottom_shapes[0].c;

    float input_scale = case_blobs[input].scale;
    float output_scale = case_blobs[output].scale;
    float weight_scale = quantize->get_scaled(ip->weight_data);
    fixed_mul_t requantize = get_fixed_mul(output_scale / (input_scale * weight_scale));

    int32_t max;
    int32_t min;
    fixed_mul_t leaky;
    switch (ip->activation_type)
    {
        case 0:
        {
            min = INT16_MIN;
            max = INT16_MAX;
            leaky = get_fixed_mul(1.f);
            break;
        }
        case 1: {
            min = 0;
            max = INT16_MAX;
            leaky = get_fixed_mul(1.f);
            break;
        }
        case 2: {
            min = INT16_MIN;
            max = INT16_MAX;
            leaky = get_fixed_mul(ip->activation_params[0]);
            break;
        }
        case 3: {
            min = ip->activation_params[0]*output_scale;
            max = ip->activation_params[1]*output_scale;
            leaky = get_fixed_mul(1.f);
            break;
        }
        default:
            printf("Convolution %s not supported activation type", ip->name.c_str());
            return false;
    }

    char buffer[1024] = {0};
    sprintf(buffer,
            "DEFINE_INNER_PRODUCT_LAYER(%s,"
            " %d, %d, %d, %d, %d,"
            " %d, %d, %d, %d, %d, %d);\n",
            ip->name.c_str(),
            1, 1, inw * inh * inc, ip->num_output, ip->bias_term,
            requantize.round_mul, requantize.shift,
            leaky.round_mul, leaky.shift,
            max, min);

    layer_define = buffer;
    return true;
}

bool InnerProductCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(inner_product, " + layer->name + ")";
    return true;
}

IMPL_CASE(InnerProduct)