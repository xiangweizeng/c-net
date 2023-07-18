/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef DETECT_OBJECT_H
#define DETECT_OBJECT_H

#include "tensor.h"

/**
 * Get affine matrix from 5 points
 * @param src_5pts
 * @param dst_5pts
 * @param M
 */
FUNCTION_IRAM void get_affine_matrix_from_5p(float* src_5pts, const float* dst_5pts, float* M);

/// Get affine matrix from 4 points
/// \param src_4pts
/// \param dst_4pts
/// \param M
FUNCTION_IRAM void get_affine_matrix_from_4p(float *src_4pts, const float *dst_4pts, float *M);

/**
 * Warp affine matrix
 * @param src
 * @param M
 * @param dst_w
 * @param dst_h
 * @return brg tensor
 */
FUNCTION_IRAM tensor_t warp_affine_matrix(tensor_t *src, float *M, int dst_w, int dst_h);


#endif /* DETECT_OBJECT_H */
