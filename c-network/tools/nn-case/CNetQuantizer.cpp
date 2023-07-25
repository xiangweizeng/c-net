/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "CNetQuantizer.h"
#include "QuantizeData.h"

#include <cmath>
#include <numeric>
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
    ncnn::Mat quantize_mat(const ncnn::Mat& float_mat, float scale)
    {
        size_t size = float_mat.total();
        ncnn::Mat quantized_data;
        quantized_data.create((int)size, sizeof(type), (ncnn::Allocator*)nullptr);

        float* input = ((float*)float_mat.data);
        type* output = ((type*)quantized_data.data);
        for (size_t i = 0; i < size; i++) {
            output[i] = float2int<type>(input[i] * scale);
        }

        return quantized_data;
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

    template<typename type>
    float get_scaled(const ncnn::Mat& float_mat, int number = 2048)
    {
        QuantizeData quantize_data("scaled", number);

        quantize_data.initial_blob_max(float_mat);
        return std::numeric_limits<type>::max() / quantize_data.max_value;
    }
};

CNetQuantizer::~CNetQuantizer() {
}

template<typename Type>
class CNetQuantizerImpl: public CNetQuantizer{
public:

    float get_scaled(const ncnn::Mat& float_mat, int number = 2048) override{
        return ::get_scaled<Type>(float_mat, number);
    }

    ncnn::Mat do_quantize(const ncnn::Mat& float_mat, float scale) override {
        return ::quantize_mat<Type>(float_mat,scale);
    };

    ncnn::Mat permute(const ncnn::Mat &mat, int n, int h, int w, int c) override {
        return ::permute<Type>(mat, n, h, w, c);
    }
};

std::shared_ptr<CNetQuantizer> CNetQuantizer::makeQuantizer(data_type_t data_type) {
    switch (data_type) {
        case int8_data_type:
            return std::make_shared<CNetQuantizerImpl<int8_t>>();
        case int16_data_type:
            return std::make_shared<CNetQuantizerImpl<int16_t>>();
        default:{
            break;
        }
    }

    return nullptr;
}
