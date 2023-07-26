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
    float input_scale = case_blobs[input].scale;
    ncnn::Mat weight_scale = quantize->get_channels_scaled(ip->weight_data);
    std::string param_var = "layer_" + ip->name + "_inner_product_filters";
    auto quantize_data = quantize->do_quantize(ip->weight_data, weight_scale);

    if(ip->bottom_shapes[0].dims == 3){
        int n = ip->num_output;
        int h = (int)ip->bottom_shapes[0].h;
        int w = (int)ip->bottom_shapes[0].w;
        int c = (int)ip->bottom_shapes[0].c;

        quantize_data = quantize->permute(quantize_data, n, h, w, c);
    }
    quantize_data_weights[param_var] = QuantizeMat(quantize_data, data_type);

    param_var = "layer_" + ip->name + "_inner_product_bias";
    if (ip->bias_term) {
        ncnn::Mat output_quantize_data;
        output_quantize_data.create(1, 4u, nullptr);
        output_quantize_data[0] = output_scale;

        quantize_data_weights[param_var] = QuantizeMat(
                quantize->do_quantize_s32(ip->bias_data, output_quantize_data),
                int32_data_type);
    } else{
        quantize_data_weights[param_var] = QuantizeMat();
    }

    param_var = "layer_" + ip->name + "_inner_product_requantize";
    for(size_t i = 0; i < weight_scale.total(); i++){
        weight_scale[i] = output_scale / (weight_scale[i] * input_scale);
    }
    quantize_data_weights[param_var] = QuantizeMat(weight_scale, float_data_type);

    return true;
}

bool InnerProductCase::get_layer_define(std::string &layer_define) {
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    auto *ip = dynamic_cast<ncnn::InnerProduct *>(layer);
    int inw = (int)ip->bottom_shapes[0].w;
    int inh = (int)ip->bottom_shapes[0].h;
    int inc = (int)ip->bottom_shapes[0].c;
    float output_scale = case_blobs[output].scale;

    int32_t max;
    int32_t min;
    float leaky;
    switch (ip->activation_type)
    {
        case 0:
        {
            min = quantize->get_number_min();
            max = quantize->get_number_max();
            leaky = (1.f);
            break;
        }
        case 1: {
            min = 0;
            max = quantize->get_number_max();
            leaky = (1.f);
            break;
        }
        case 2: {
            min = quantize->get_number_min();
            max = quantize->get_number_max();
            leaky = (ip->activation_params[0]);
            break;
        }
        case 3: {
            min = quantize->float2int(ip->activation_params[0]*output_scale);
            max = quantize->float2int(ip->activation_params[1]*output_scale);
            leaky = (1.f);
            break;
        }
        default:
            printf("Convolution %s not supported activation type", ip->name.c_str());
            return false;
    }

    char buffer[1024] = {0};
    int req_num = 1;
    sprintf(buffer,
            "DEFINE_INNER_PRODUCT_LAYER(%s,"
            " %d, %d, %d, %d, %d,"
            " %d, %f, %d, %d);\n",
            ip->name.c_str(),
            1, 1, inw * inh * inc, ip->num_output, ip->bias_term,
            req_num, leaky, max, min);

    layer_define = buffer;
    return true;
}

bool InnerProductCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(inner_product, " + layer->name + ")";
    return true;
}

IMPL_CASE(InnerProduct)