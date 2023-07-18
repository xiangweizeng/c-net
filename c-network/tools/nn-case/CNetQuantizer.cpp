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
