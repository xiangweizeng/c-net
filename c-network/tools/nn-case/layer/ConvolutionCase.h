/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_CONVOLUTION_CASE_H
#define CNET_CONVOLUTION_CASE_H

#include "../CNetLayerCase.h"

class ConvolutionCase : public CNetLayerCase{
public:
    ConvolutionCase(CNetCase* network, ncnn::Layer *layer);

    bool ignore() override;
    bool quantize_weights() override;
    bool get_layer_define(std::string &layer_define) override;
    bool get_layer_reference(std::string &reference) override;

protected:

    template<class Conv>
    bool quantize_convolution(
            const Conv* convolution,
            int group);

    template<class Conv>
    bool case_convolution(
            const Conv* convolution,
            int group,
            std::string &layer_define);
};


#endif //CNET_CONVOLUTION_CASE_H
