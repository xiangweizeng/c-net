/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_CROP_CASE_H
#define CNET_CROP_CASE_H

#include "../CNetLayerCase.h"

class CropCase : public CNetLayerCase{
public:
    CropCase(CNetCase *network, ncnn::Layer *layer);

    bool ignore() override;

    bool quantize_weights() override;

    bool get_layer_define(std::string &layer_define) override;

    bool get_layer_reference(std::string &reference) override;

    void crop_roi(const ncnn::Crop *crop, int dims, int &_woffset, int &_hoffset, int &_coffset, int &_outw, int &_outh,
                  int &_outc) const;
};


#endif //CNET_CROP_CASE_H
