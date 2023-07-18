/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_QUANTIZE_H
#define CNET_QUANTIZE_H

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "allocator.h"
#include "option.h"
#include <tensor.h>

FUNCTION_IRAM static inline int8_t float2int8(float v) {
    int int32 = round(v);
    if (int32 > 127) return 127;
    if (int32 < -128) return -128;
    return (int8_t) int32;
}

FUNCTION_IRAM static inline uint8_t float2uint8(float v) {
    int32_t int32 = round(v);
    if (int32 > 255) return 255;
    if (int32 < 0) return 0;
    return (uint8_t) int32;
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

typedef  struct fixed_mul_t
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
    const int32_t multiplier = ((int32_t) scale_bits & INT32_C(0x007FFFFF)) | INT32_C(0x00800000);
    const int32_t shift = 127 + 23 - ((scale_bits >> 23) & 0xFF);

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
#define CLIP_NORMAL(v, out_max, out_min) ((v = v <= out_max ? v : out_max) <= out_min ? out_min : v)
#define DEQUANTIZE_BIAS(v, s, b) ((v)*(s) + b)

FUNCTION_IRAM static inline int16_t float2int16(float v) {
    int int32 = round(v);
    if (int32 > 32767) return 32767;
    if (int32 < -32768) return -32768;
    return (int16_t)int32;
}

#define CLIP_INT16(v, out_max, out_min) (int16_t)((v = v <= out_max ? v : out_max) <= out_min ? out_min : v)

/**
 * quantize tensor, parallize
 * top_tensor must have allocated memory, and set data type,
 * @param bottom_tensor
 * @param top_tensor
 * @param param
 * @param opt
 */
FUNCTION_IRAM void quantize16_tensor_parallize(tensor_t* bottom_tensor, tensor_t* top_tensor, float scale, option_t* opt);

#endif //CNET_QUANTIZE_H
