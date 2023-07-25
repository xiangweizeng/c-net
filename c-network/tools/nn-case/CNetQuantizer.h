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
class CNetQuantizer {
public:

    /// ~CNetQuantize
    virtual ~CNetQuantizer();

    /// Do quantize
    /// \param float_mat
    /// \param scale
    /// \return
    virtual ncnn::Mat do_quantize(const ncnn::Mat& float_mat, float scale) = 0;

    /// Permute
    /// \param mat
    /// \param n
    /// \param h
    /// \param w
    /// \param c
    /// \return
    virtual ncnn::Mat permute(const ncnn::Mat& mat, int n, int h, int w, int c) = 0;

    /// Get Scaled
    /// \param float_mat
    /// \param number
    /// \return
    virtual float get_scaled(const ncnn::Mat& float_mat, int number = 2048) = 0;

    /// Make Quantizer
    /// \param data_type
    /// \return
    static std::shared_ptr<CNetQuantizer> makeQuantizer(data_type_t data_type);
};


#endif //CNET_CNET_QUANTIZE_H
