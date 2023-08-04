/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_QUANTIZE_S8_H
#define CNET_QUANTIZE_S8_H

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "allocator.h"
#include "option.h"
#include <tensor.h>
#include <assert.h>

FUNCTION_IRAM static inline int8_t float2int8(float v) {
    int int32 = round(v);
    if (int32 > 127) return 127;
    if (int32 < -128) return -128;
    return (int8_t) int32;
}

FUNCTION_IRAM static inline float fp32_from_bits(uint32_t w) {
    union {
        uint32_t as_bits;
        float as_value;
    } fp32 = { w };
    return fp32.as_value;
}

FUNCTION_IRAM static inline uint32_t fp32_to_bits(float f) {
    union {
        float as_value;
        uint32_t as_bits;
    } fp32 = { f };
    return fp32.as_bits;
}

typedef struct fixed_mul_t
{
    int32_t round_mul;
    int8_t shift;
}fixed_mul_t;

FUNCTION_IRAM static fixed_mul_t get_fixed_mul(float value)
{
    if(value == 0){
        fixed_mul_t fixed_mul = {0, (int8_t)(0) };
        return  fixed_mul;
    }
    uint32_t scale_bits = fp32_to_bits(value);
    const int32_t multiplier = (((int32_t) scale_bits & INT32_C(0x007FFFFF)) | INT32_C(0x00800000)) >> 19;
    const int32_t shift = 127 + 4 - ((scale_bits >> 23) & 0xFF);

    assert(shift >= 0);

    if(value > 0){
        fixed_mul_t fixed_mul = {multiplier, (int8_t)(shift) };
        return  fixed_mul;
    } else{
        fixed_mul_t fixed_mul = {-multiplier, (int8_t)(shift) };
        return  fixed_mul;
    }
}

#define MULTIPLY_FIDED(v, fixedmul) (((int64_t )v*fixedmul.round_mul) >> fixedmul.shift)
#define REQUANTIZE_BIAS(v, fixedmul, bias_value) ((((int64_t )v*fixedmul.round_mul) >> fixedmul.shift) + bias_value)
#define CLIP_INT8(v, out_max, out_min) (int8_t)((v = v <= out_max ? v : out_max) <= out_min ? out_min : v)

/**
 * Quantize tensor
 * top_tensor must have allocated memory, and set data type,
 * @param bottom_tensor
 * @param top_tensor
 * @param param
 * @param opt
 */
FUNCTION_IRAM void quantize_tensor(
        tensor_t* bottom_tensor,
        tensor_t* top_tensor,
        float scale,
        option_t* opt);

#endif //CNET_QUANTIZE_S8_H
