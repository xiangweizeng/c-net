/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_CNET_QUANTIZE_H
#define CNET_CNET_QUANTIZE_H

#include <memory>
#include "mat.h"
#include "operation_config.h"

/**
 * CNetQuantize
 */
class CNetQuantize {
public:

    /**
     * ~CNetQuantize
     */
    virtual ~CNetQuantize();

    /**
     * Get number min
     * @return
     */
    virtual int32_t get_number_min() = 0;

    /**
     * Get number max
     * @return
     */
    virtual int32_t get_number_max() = 0;

    /**
     * Float to int, clip to min/max
     * @param v
     * @return
     */
    virtual int32_t float2int(float v) = 0;

    /**
     * Get channels scaled
     * @param float_mat
     * @param channels
     * @param number
     * @return
     */
    virtual ncnn::Mat get_channels_scaled(const ncnn::Mat& float_mat, int channels = 1,  int number = 2048) = 0;

    /// Do quantize
    /// \param float_mat
    /// \param scale
    /// \return
    virtual ncnn::Mat do_quantize(const ncnn::Mat& float_mat, ncnn::Mat scale, int channels = 1) = 0;

    /// Do quantize
    /// \param float_mat
    /// \param scale
    /// \return
    virtual ncnn::Mat do_quantize_s32(const ncnn::Mat& float_mat, ncnn::Mat scale, int channels = 1) = 0;

    /// Permute
    /// \param mat
    /// \param n
    /// \param h
    /// \param w
    /// \param c
    /// \return
    virtual ncnn::Mat permute(const ncnn::Mat& mat, int n, int h, int w, int c) = 0;

    /// Make Quantize
    /// \param data_type
    /// \return
    static std::shared_ptr<CNetQuantize> makeQuantize(data_type_t data_type);
};

/**
 * QuantizeMat
 */
class QuantizeMat{
public:
    QuantizeMat() = default;

    QuantizeMat(const ncnn::Mat &data, data_type_t data_type):
            data(data), data_type(data_type){
        switch (data_type) {
            case float_data_type:
                extra = 128;
                break;
            case int8_data_type:
                extra = 32;
                break;
            case int16_data_type:
                extra = 64;
                break;
            case int32_data_type:
                extra = 128;
                break;
        }
    }

public:
    /// Data
    ncnn::Mat data;

    /// Data type
    data_type_t data_type;

    /// Extra in trail
    int extra;

};

#endif //CNET_CNET_QUANTIZE_H
