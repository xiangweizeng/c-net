/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/convolution.h>
#include <layer/convolutiondepthwise.h>
#include "ConvolutionCase.h"

template<class ConvType>
static std::vector<int> get_conv_padding(ConvType* convolution, int w, int h)
{
    int kernel_w = convolution->kernel_w;
    int kernel_h = convolution->kernel_h;
    int dilation_w = convolution->dilation_w;
    int dilation_h = convolution->dilation_h;
    int stride_w = convolution->stride_w;
    int stride_h = convolution->stride_h;

    int pad_right = convolution->pad_right;
    int pad_left = convolution->pad_left;
    int pad_bottom = convolution->pad_bottom;
    int pad_top = convolution->pad_top;

    const int kernel_extent_w = dilation_w * (kernel_w - 1) + 1;
    const int kernel_extent_h = dilation_h * (kernel_h - 1) + 1;

    if (pad_left == -233 && pad_right == -233 && pad_top == -233 && pad_bottom == -233)
    {
        // tensorflow padding=SAME or onnx padding=SAME_UPPER
        pad_right = pad_bottom = pad_left = pad_top = 0;
        int wpad = kernel_extent_w + (w - 1) / stride_w * stride_w - w;
        int hpad = kernel_extent_h + (h - 1) / stride_h * stride_h - h;
        if (wpad > 0 || hpad > 0)
        {
            pad_top = hpad / 2;
            pad_bottom = hpad - hpad / 2;
            pad_left = wpad / 2;
            pad_right = wpad - wpad / 2;
        }
    }
    else if (pad_left == -234 && pad_right == -234 && pad_top == -234 && pad_bottom == -234)
    {
        // onnx padding=SAME_LOWER
        pad_right = pad_bottom = pad_left = pad_top = 0;
        int wpad = kernel_extent_w + (w - 1) / stride_w * stride_w - w;
        int hpad = kernel_extent_h + (h - 1) / stride_h * stride_h - h;
        if (wpad > 0 || hpad > 0)
        {
            pad_top = hpad - hpad / 2;
            pad_bottom = hpad / 2;
            pad_left = wpad - wpad / 2;
            pad_right = wpad / 2;
        }
    }

    return { pad_left, pad_right , pad_top, pad_bottom };
}


template<class Conv>
bool ConvolutionCase::quantize_convolution(const Conv* convolution, int group)
{
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    float output_scale = case_blobs[output].scale;
    float input_scale = case_blobs[input].scale;
    ncnn::Mat weight_scale = quantize->get_channels_scaled(convolution->weight_data);
    std::string param_var = "layer_" + convolution->name + "_convolution_filters";
    ncnn::Mat quantize_data = quantize->do_quantize(convolution->weight_data, weight_scale);
    if(group == 1){
        quantize_data = quantize->permute(
                quantize_data,
                convolution->num_output,
                convolution->kernel_h,
                convolution->kernel_w,
                (int)convolution->bottom_shapes[0].c);
    } else{

        int n = convolution->num_output / group;
        int h = convolution->kernel_h;
        int w = convolution->kernel_w;
        int c = (int)convolution->bottom_shapes[0].c / group;
        int group_filters_size = n * h * w * c;

        for(int g = 0; g < group; g++){
            auto group_mat = quantize_data.range(g * group_filters_size, group_filters_size);
            auto permute_mat = quantize->permute(group_mat, n, h, w, c);
            memcpy(group_mat.data, permute_mat.data, group_filters_size * group_mat.elemsize);
        }
    }
    quantize_data_weights[param_var] = QuantizeMat(quantize_data, data_type);

    param_var = "layer_" + convolution->name + "_convolution_bias";
    if (convolution->bias_term) {
        ncnn::Mat output_quantize_data;
        output_quantize_data.create(1, 4u, nullptr);
        output_quantize_data[0] = output_scale;

        quantize_data_weights[param_var] = QuantizeMat(
                quantize->do_quantize_s32(convolution->bias_data, output_quantize_data),
                int32_data_type);
    } else{
        quantize_data_weights[param_var] = QuantizeMat();
    }

    param_var = "layer_" + convolution->name + "_convolution_requantize";
    for(int i = 0; i < weight_scale.total(); i++){
        weight_scale[i] = output_scale / (weight_scale[i] * input_scale);
    }
    quantize_data_weights[param_var] = QuantizeMat(weight_scale, float_data_type);

    return true;
}

template<class Conv>
bool ConvolutionCase::case_convolution(const Conv* convolution, int group, std::string &layer_define)
{
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    int inw = (int)convolution->bottom_shapes[0].w;
    int inh = (int)convolution->bottom_shapes[0].h;
    int16_t pad_value = convolution->pad_value;
    std::vector<int> padding = get_conv_padding(convolution, inw, inh);

    float input_scale = case_blobs[input].scale;
    float output_scale = case_blobs[output].scale;

    int32_t max;
    int32_t min;
    float leaky;
    switch (convolution->activation_type)
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
            leaky = (convolution->activation_params[0]);
            break;
        }
        case 3: {
            min = quantize->float2int(convolution->activation_params[0]*output_scale);
            max = quantize->float2int(convolution->activation_params[1]*output_scale);
            leaky = (1.f);
            break;
        }
        default:
            printf("Convolution %s not supported activation type", convolution->name.c_str());
            return false;
    }

    char buffer[1024] = {0};
    int32_t req_num = 1;
    sprintf(buffer,
            "DEFINE_CONVOLUTION_LAYER(%s,"
            " %d, %d, %d, %d,"
            " %d,"
            " %d, %d, %d, %d, "
            " %d, %d, %d, %d, %d,"
            " %d, %d, %f, %d, %d);\n",
            convolution->name.c_str(),
            convolution->num_output,
            convolution->kernel_h,
            convolution->kernel_w,
            (int)convolution->bottom_shapes[0].c,
            group,
            convolution->stride_w, convolution->stride_h, convolution->dilation_w, convolution->dilation_h,
            padding[0], padding[1], padding[2], padding[3], pad_value,
            convolution->bias_term, req_num, leaky, max, min);

    layer_define = buffer;

    return true;
}

bool ConvolutionCase::ignore() {
    return false;
}

bool ConvolutionCase::quantize_weights() {
    if (layer->type == "Convolution") {
        const auto* convolution = dynamic_cast<const ncnn::Convolution*>(layer);
        return quantize_convolution(convolution, 1);
    }
    else if (layer->type == "ConvolutionDepthWise") {
        const auto* convolution = dynamic_cast<const ncnn::ConvolutionDepthWise*>(layer);
        return quantize_convolution(convolution, convolution->group);
    }
    return false;
}

bool ConvolutionCase::get_layer_define(std::string &layer_define) {
    if (layer->type == "Convolution") {
        const auto* convolution = dynamic_cast<const ncnn::Convolution*>(layer);
        return case_convolution(convolution, 1, layer_define);
    }
    else if (layer->type == "ConvolutionDepthWise") {
        const auto* convolution = dynamic_cast<const ncnn::ConvolutionDepthWise*>(layer);
        return case_convolution(convolution, convolution->group, layer_define);
    }

    return false;
}

bool ConvolutionCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(convolution, " + layer->name + ")";
    return true;
}

ConvolutionCase::ConvolutionCase(CNetCase* network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

static PCNetLayerCase CaseCreator(CNetCase* network, ncnn::Layer *layer){
    return std::make_shared<ConvolutionCase>(network, layer);
}

REGISTER_CASE(Convolution, CaseCreator)
REGISTER_CASE(ConvolutionDepthWise, CaseCreator)
