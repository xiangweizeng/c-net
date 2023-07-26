/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/crop.h>
#include "CropCase.h"

#undef Min
#undef Max
#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))

CropCase::CropCase(CNetCase *network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool CropCase::ignore() {
    return false;
}

bool CropCase::quantize_weights() {
    return true;
}

bool CropCase::get_layer_define(std::string &layer_define) {

    auto *crop = dynamic_cast<ncnn::Crop *>(layer);
    std::string input = get_blob_input_name(0);
    std::string output = get_blob_output_name(0);

    float input_scale = case_blobs[input].scale;
    float output_scale = case_blobs[output].scale;
    fixed_mul_t requantize = get_fixed_mul(output_scale / input_scale);

    int dims = crop->bottom_shapes[0].dims;;
    int _woffset = crop->woffset;
    int _hoffset = crop->hoffset;
    int _coffset = crop->coffset;
    int _outw = 0;
    int _outh = 0;
    int _outc = 0;

    crop_roi(crop, dims, _woffset, _hoffset, _coffset, _outw, _outh, _outc);

    char buffer[1024] = {0};
    sprintf(buffer, "DEFINE_CROP_LAYER(%s,%d, %d, %d, %d, %d, %d, %d);\n",
            crop->name.c_str(),
            dims, _woffset, _hoffset, _coffset, _outw, _outh, _outc);
    layer_define = buffer;
    return true;
}

void CropCase::crop_roi(const ncnn::Crop *crop, int dims, int &_woffset, int &_hoffset, int &_coffset, int &_outw,
                        int &_outh, int &_outc) const {

    int w = crop->bottom_shapes[0].w;
    int h = crop->bottom_shapes[0].h;
    int channels = crop->bottom_shapes[0].c;

    bool numpy_style_slice = !crop->starts.empty() && !crop->ends.empty();
    if (numpy_style_slice)
    {
        _woffset = 0;
        _hoffset = 0;
        _coffset = 0;
        _outw = w;
        _outh = h;
        _outc = channels;

        const int* starts_ptr = crop->starts;
        const int* ends_ptr = crop->ends;
        const int* axes_ptr = crop->axes;

        int _axes[4] = {0, 1, 2, 3};
        int num_axis = crop->axes.w;
        if (num_axis == 0)
        {
            num_axis = dims;
        }
        else
        {
            for (int i = 0; i < num_axis; i++)
            {
                int axis = axes_ptr[i];
                if (axis < 0)
                    axis = dims + axis;
                _axes[i] = axis;
            }
        }

        for (int i = 0; i < num_axis; i++)
        {
            int axis = _axes[i];
            int start = starts_ptr[i];
            int end = ends_ptr[i];

            if (dims == 1) // axis == 0
            {
                if (start == -233) start = 0;
                if (end == -233) end = w;
                _woffset = start >= 0 ? start : w + start;
                _outw = std::min(w, end > 0 ? end : w + end) - _woffset;
            }
            if (dims == 2)
            {
                if (axis == 0)
                {
                    if (start == -233) start = 0;
                    if (end == -233) end = h;
                    _hoffset = start >= 0 ? start : h + start;
                    _outh = std::min(h, end > 0 ? end : h + end) - _hoffset;
                }
                if (axis == 1)
                {
                    if (start == -233) start = 0;
                    if (end == -233) end = w;
                    _woffset = start >= 0 ? start : w + start;
                    _outw = std::min(w, end > 0 ? end : w + end) - _woffset;
                }
            }
            if (dims == 3)
            {
                if (axis == 0)
                {
                    if (start == -233) start = 0;
                    if (end == -233) end = channels;
                    _coffset = start >= 0 ? start : channels + start;
                    _outc = std::min(channels, end > 0 ? end : channels + end) - _coffset;
                }
                if (axis == 1)
                {
                    if (start == -233) start = 0;
                    if (end == -233) end = h;
                    _hoffset = start >= 0 ? start : h + start;
                    _outh = std::min(h, end > 0 ? end : h + end) - _hoffset;
                }
                if (axis == 2)
                {
                    if (start == -233) start = 0;
                    if (end == -233) end = w;
                    _woffset = start >= 0 ? start : w + start;
                    _outw = std::min(w, end > 0 ? end : w + end) - _woffset;
                }
            }
        }
    }
    else
    {
        _woffset = crop->woffset;
        _hoffset = crop->hoffset;
        _coffset = crop->coffset;
        _outw = w;
        _outh = h;
        _outc = channels;

        if (dims == 1)
        {
            _outw = w - crop->woffset - crop->woffset2;
            if (crop->outw != -233)
                _outw = std::min(crop->outw, _outw);
        }
        if (dims == 2)
        {
            _outw = w - crop->woffset - crop->woffset2;
            if (crop->outw != -233)
                _outw = std::min(crop->outw, _outw);

            _outh = h - crop->hoffset - crop->hoffset2;
            if (crop->outh != -233)
                _outh = std::min(crop->outh, _outh);
        }
        if (dims == 3)
        {
            _outw = w - crop->woffset - crop->woffset2;
            if (crop->outw != -233)
                _outw = std::min(crop->outw, _outw);

            _outh = h - crop->hoffset - crop->hoffset2;
            if (crop->outh != -233)
                _outh = std::min(crop->outh, _outh);

            _outc = channels - crop->coffset - crop->coffset2;
            if (crop->outc != -233)
                _outc = std::min(crop->outc, _outc);
        }
    }
}

bool CropCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(crop, " + layer->name + ")";
    return true;
}

IMPL_CASE(Crop)