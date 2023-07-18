/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_TENSOR_OPERATION_H
#define CNET_TENSOR_OPERATION_H

#include "tensor.h"
#include "option.h"

/// Softmax
/// \param axis
/// \param inplace_tensor
/// \param opt
/// \return
int tensor_softmax(int axis, tensor_t *inplace_tensor, option_t *opt);

/**
 * Border type
 */
typedef enum border_type_t
{
    BORDER_CONSTANT = 0,
    BORDER_REPLICATE = 1,
}border_type_t;

/**
 * Tensor Padding
 * @param src
 * @param top
 * @param bottom
 * @param left
 * @param right
 * @param type
 * @param v
 * @param opt
 * @return
 */
FUNCTION_IRAM tensor_t tensor_padding(
        tensor_t *src,
        int top, int bottom,
        int left, int right,
        border_type_t type,
        float v,
        option_t *opt);

/**
 * Tensor cut
 * @param src
 * @param top
 * @param bottom
 * @param left
 * @param right
 * @param opt
 */
FUNCTION_IRAM tensor_t tensor_cut_border(
        tensor_t *src,
        int top, int bottom,
        int left, int right,
        option_t *opt);

/**
 * Tensor resize by bilinear
 * @param src
 * @param w
 * @param h
 * @param opt
 * @return
 */
FUNCTION_IRAM tensor_t tensor_resize_bilinear(
        tensor_t *src,
        int w, int h,
        option_t* opt);

/**
 * Tensor resize bicubic
 * @param src
 * @param w
 * @param h
 * @param opt
 * @return
 */
FUNCTION_IRAM tensor_t tensor_resize_bicubic(
        tensor_t *src,
        int w, int h,
        option_t *opt);


#endif //CNET_TENSOR_OPERATION_H
