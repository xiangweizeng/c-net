/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "CNetQuantize.h"
#include "QuantizeData.h"

#include <cmath>
#include <limits>


namespace {
    template<typename type>
    inline type float2int(float v) {
        int int32 = round(v);
        if (int32 > std::numeric_limits<type>::max()){
            return std::numeric_limits<type>::max();
        }

        if (int32 < std::numeric_limits<type>::min()){
            return std::numeric_limits<type>::min();
        }
        return (type)int32;
    }

    template<typename type>
    float get_scaled(const ncnn::Mat& float_mat, int number = 2048)
    {
        QuantizeData quantize_data("scaled", number);

        quantize_data.initial_blob_max(float_mat);
        return std::numeric_limits<type>::max() / quantize_data.max_value;
    }

    template<typename type>
    void quantize_mat(const ncnn::Mat& float_mat, ncnn::Mat &quantize_data, float scale)
    {
        size_t size = float_mat.total();
        if(quantize_data.empty()){
            quantize_data.create((int)size, sizeof(type), (ncnn::Allocator*)nullptr);
        }

        float* input = ((float*)float_mat.data);
        type* output = ((type*)quantize_data.data);
        for (size_t i = 0; i < size; i++) {
            output[i] = float2int<type>(input[i] * scale);
        }
    }

    template<typename type>
    ncnn::Mat permute(const ncnn::Mat& mat, int n, int h, int w, int c){
        int c_step = w * h;

        ncnn::Mat output_mat = mat.clone();
        for(int n_ = 0; n_ < n; n_ ++){
            type *input = (type*)mat.data + n_ * w * h * c;
            for(int h_ = 0; h_ < h; h_ ++){

                type *output = (type*)output_mat.data + n_ * w * h * c + h_*w*c;
                for(int w_ = 0; w_ < w; w_ ++){
                    for(int c_ = 0; c_ < c; c_++){
                        type *input_c = input + c_ * c_step + h_ * w + w_;
                        *output++ = *input_c;
                    }
                }
            }
        }

        return output_mat;
    }
};

CNetQuantize::~CNetQuantize() {
}

template<typename Type>
class CNetQuantizeImpl: public CNetQuantize{
public:

    int32_t get_number_min() override {
        return std::numeric_limits<Type>::min();
    }

    int32_t get_number_max() override {
        return std::numeric_limits<Type>::max();
    }

    int32_t float2int(float v) override {
        return ::float2int<Type>(v);
    }

    ncnn::Mat get_channels_scaled(const ncnn::Mat &float_mat, int channels = 1, int number = 2048) override{
        ncnn::Mat scaled_mat;
        scaled_mat.create(channels, 4u, nullptr);

        int group_size = float_mat.total() / channels;
        auto *scaled = static_cast<float *>(scaled_mat.data);
        for(int i = 0; i < channels; i++){
            ncnn::Mat c_mat = float_mat.range(i * group_size, group_size);
            scaled[i] = get_scaled<Type>(c_mat);
        }

        return scaled_mat;
    }

    ncnn::Mat do_quantize(const ncnn::Mat &float_mat, ncnn::Mat scaled_mat, int channels = 1) override{

        int group_size = float_mat.total() / channels;
        auto *scaled = static_cast<float *>(scaled_mat.data);

        ncnn::Mat quantize_data;
        quantize_data.create(float_mat.total(), sizeof(Type), nullptr);
        for(int i = 0; i < channels; i++){
            ncnn::Mat c_mat = float_mat.range(i * group_size, group_size);
            ncnn::Mat q_mat = quantize_data.range(i * group_size, group_size);
            quantize_mat<Type>(c_mat, q_mat, scaled[i]);
        }

        return quantize_data;
    }

    ncnn::Mat do_quantize_s32(const ncnn::Mat &float_mat, ncnn::Mat scaled_mat, int channels = 1) override{
        int group_size = float_mat.total() / channels;
        auto *scaled = static_cast<float *>(scaled_mat.data);

        ncnn::Mat quantize_data;
        quantize_data.create(float_mat.total(), sizeof(int32_t), nullptr);
        for(int i = 0; i < channels; i++){
            ncnn::Mat c_mat = float_mat.range(i * group_size, group_size);
            ncnn::Mat q_mat = quantize_data.range(i * group_size, group_size);
            quantize_mat<int32_t>(c_mat, q_mat, scaled[i]);
        }

        return quantize_data;
    }

    ncnn::Mat permute(const ncnn::Mat &mat, int n, int h, int w, int c) override {
        return ::permute<Type>(mat, n, h, w, c);
    }
};

std::shared_ptr<CNetQuantize> CNetQuantize::makeQuantize(data_type_t data_type) {
    switch (data_type) {
        case int8_data_type:
            return std::make_shared<CNetQuantizeImpl<int8_t>>();
        case int16_data_type:
            return std::make_shared<CNetQuantizeImpl<int16_t>>();
        default:{
            break;
        }
    }

    return nullptr;
}

