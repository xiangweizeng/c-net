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
bool ConvolutionCase::quantize_convolution(const Conv* convolution)
{
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    float output_scale = case_blobs[output].scale;
    float weight_scale = quantize->get_scaled(convolution->weight_data);
    std::string param_var = "layer_" + convolution->name + "_convolution_filters";
    quantize_data_weights[param_var] = quantize->do_quantize(convolution->weight_data, weight_scale);

    param_var = "layer_" + convolution->name + "_convolution_bias";
    if (convolution->bias_term) {
        quantize_data_weights[param_var] = quantize->do_quantize(convolution->bias_data, output_scale);
    } else{
        quantize_data_weights[param_var] = ncnn::Mat();
    }

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
    float weight_scale = quantize->get_scaled(convolution->weight_data);
    fixed_mul_t requantize = get_fixed_mul(output_scale / (input_scale * weight_scale));

    int32_t max;
    int32_t min;
    fixed_mul_t leaky;
    switch (convolution->activation_type)
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
            leaky = get_fixed_mul(convolution->activation_params[0]);
            break;
        }
        case 3: {
            min = convolution->activation_params[0]*output_scale;
            max = convolution->activation_params[1]*output_scale;
            leaky = get_fixed_mul(1.f);
            break;
        }
        default:
            printf("Convolution %s not supported activation type", convolution->name.c_str());
            return false;
    }

    char buffer[1024] = {0};
    sprintf(buffer,
            "DEFINE_CONVOLUTION_LAYER(%s,"
            " %d, %d, %d, %d,"
            " %d,"
            " %d, %d, %d, %d, "
            " %d, %d, %d, %d, %d,"
            " %d, %d, %d, %d, %d, %d, %d);\n",
            convolution->name.c_str(),
            convolution->num_output,
            (int)convolution->bottom_shapes[0].c,
            convolution->kernel_h,
            convolution->kernel_w,
            group,
            convolution->stride_w, convolution->stride_h, convolution->dilation_w, convolution->dilation_h,
            padding[0], padding[1], padding[2], padding[3], pad_value,
            convolution->bias_term, requantize.round_mul, requantize.shift, leaky.round_mul, leaky.shift, max, min);

    layer_define = buffer;

    return true;
}

bool ConvolutionCase::ignore() {
    return false;
}

bool ConvolutionCase::quantize_weights() {
    if (layer->type == "Convolution") {
        const auto* convolution = dynamic_cast<const ncnn::Convolution*>(layer);
        return quantize_convolution(convolution);
    }
    else if (layer->type == "ConvolutionDepthWise") {
        const auto* convolution = dynamic_cast<const ncnn::ConvolutionDepthWise*>(layer);
        return quantize_convolution(convolution);
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
