
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_LAYER_CONFIG_H
#define CNET_LAYER_CONFIG_H

#include "quantize16.h"


/// operation type
typedef enum operation_type {
    /// convolution
    convolution_operation_type = 0,
    /// pooling
    pooling_operation_type,
    /// padding
    padding_operation_type,
    /// crop
    crop_operation_type,
    /// inner product
    inner_product_operation_type,
    /// activation
    activation_operation_type,
    /// batch norm
    batch_norm_operation_type,
    /// slice
    slice_operation_type,
    /// concat
    concat_operation_type,
    /// reshape
    reshape_operation_type,
    /// sigmoid
    sigmoid_operation_type,
    /// softmax
    softmax_operation_type,
    /// permute
    permute_operation_type,
    /// unary
    unary_operation_type,
    /// binary
    binary_operation_type,
    /// upsample
    upsample_operation_type,
    /// prelu
    prelu_operation_type,
    /// memory data
    memory_data_operation_type,
    /// shuffle_channel
    shuffle_channel_operation_type,
} operation_type_t;

/// Operation names
extern const char* operation_names[];

/// activate type
typedef enum activate_type {
    /// bnll
    bnll_activate_type,
    /// clip
    clip_activate_type,
    /// elu
    elu_activate_type,
    /// exp
    exp_activate_type,
    /// hard sigmoid
    hard_sigmoid_activate_type,
    /// hard swish
    hard_swish_activate_type,
    /// log
    log_activate_type,
    /// power
    power_activate_type,
    /// relu
    relu_activate_type,
    /// selu
    selu_activate_type,
    /// hard swish
    swish_activate_type,
    /// threshold
    threshold_activate_type,
} activate_type_t;

/// tensor shape
typedef int blob_shape_t[4];

#define BLOB_SHAPE_OFFSET(shape, index) (((index[0] * shape[1] + index[1]) * shape[2] + index[2]) * shape[3] + index[3])
#define BLOB_SHAPE_EQUALS(a, b) (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3])

/// data type
typedef enum data_type{
    /// float
    float_data_type = 1,
    /// uint8
    int8_data_type = 2,
    /// int16
    int16_data_type = 3,
}data_type_t;

/// blob info
typedef struct blob_info {
    /// shape
    blob_shape_t shape;

    /// data type
    data_type_t data_type;

    /// consumers count
    int consumers_count;

    /// scale
    float scale;
} blob_info_t;

/// Operation blob
typedef struct operation_blobs{
    int input_count;
    int *input_blobs;
    int output_count;
    int *output_blobs;
}operation_blobs_t;

/// config for
typedef struct convolution_config {
    /**
     * operation type
     */
    operation_type_t type;

    /// filter size
    blob_shape_t filers_size;

    /// group
    int group;

    /// stride w
    int stride_w;

    /// stride h
    int stride_h;

    /// dilation
    int dilation_w;
    int dilation_h;

    /// pad mode
    int pad_left;
    int pad_right;
    int pad_top;
    int pad_bottom;
    int pad_value;

    /// have bias
    int bias_term;

    /// requantize
    fixed_mul_t requantize;

    /// float leaky
    fixed_mul_t leaky;

    /// clip max
    int32_t max;

    /// clip max
    int32_t min;
} convolution_config_t;

/// pooling type
typedef enum pool_method_type {
    pool_method_max_type = 0,
    pool_method_avg_type = 1
} pool_method_type_t;

/// pooling config
typedef struct pooling_config {
    /// operation type
    operation_type_t type;

    /// pooling type
    pool_method_type_t pooling_type;

    /// kernel w
    int kernel_w;

    /// kernel h
    int kernel_h;

    /// stride w
    int stride_w;

    /// stride h
    int stride_h;

    /// global pooling
    int global_pooling;

    /// pad_mode
    int pad_left;
    int pad_right;
    int pad_top;
    int pad_bottom;
    int pad_value;

    /// count pad
    int count_include_pad;

    /// requantize
    fixed_mul_t requantize;
} pooling_config_t;

/// padding mode
typedef enum{
    padding_mode_constant = 0,
    padding_mode_replicate = 1,
    padding_mode_reflect = 2
}padding_mode_t;

/// config for padding
typedef struct padding_config {

    /// operation type
    operation_type_t type;

    /// stride w
    int stride_w;

    /// stride_h
    int stride_h;

    /// padding mode
    padding_mode_t pad_mode;

    /// pad mode
    int pad_left;
    int pad_right;
    int pad_top;
    int pad_bottom;
    int pad_value;
} padding_config_t;

typedef struct crop_config{
    /// operation type
    operation_type_t type;

    /// dims
    int dims;

    /// offset
    int w_offset;
    int h_offset;
    int c_offset;

    /// out shape
    int out_w;
    int out_h;
    int out_c;

    /// requantize
    fixed_mul_t requantize;
}crop_config_t;

typedef struct inner_product_config {
    /// operation type
    operation_type_t type;

    /// filter size
    blob_shape_t filers_size;

    /// have bias
    int bias_term;

    /// requantize
    fixed_mul_t requantize;

    /// float leaky
    fixed_mul_t leaky;

    /// clip max
    int32_t max;

    /// clip max
    int32_t min;
} inner_product_config_t;

typedef struct activate_config {
    /// operation type
    operation_type_t type;

    /// activate type
    activate_type_t activate_type;

    /// activate params
    float activate_params[8];
} activate_config_t;

/// batch borm config
typedef struct batch_norm_config{
    /// operation type
    operation_type_t type;

    /// requantize
    fixed_mul_t requantize;
}batch_norm_config_t;

typedef struct slice_config {
    /// operation type
    operation_type_t type;

    /// axis
    int axis;

    /// requantize
    fixed_mul_t requantize;
} slice_config_t;

typedef struct concat_config {
    /// operation type
    operation_type_t type;

    /// axis
    int axis;

    /// concat count
    int concat_count;

    /// output fixed_mul
    int output_fixed_mul[24];
} concat_config_t;

typedef struct reshape_config {
    /// operation type
    operation_type_t type;
} reshape_config_t;

typedef struct sigmoid_config {
    /// operation type
    operation_type_t type;
} sigmoid_config_t;

/// soft max
typedef struct softmax_config {
    /// operation type
    operation_type_t type;

    /// axis
    int axis;
} softmax_config_t;

/// permute
typedef struct permute_config {
    /// operation type
    operation_type_t type;

    /// axis info
    blob_shape_t axis;
} permute_config_t;

typedef enum unary_type
{
    UNARY_OPERATION_ABS = 0,
    UNARY_OPERATION_NEG = 1,
    UNARY_OPERATION_FLOOR = 2,
    UNARY_OPERATION_CEIL = 3,
    UNARY_OPERATION_SQUARE = 4,
    UNARY_OPERATION_SQRT = 5,
    UNARY_OPERATION_RSQRT = 6,
    UNARY_OPERATION_EXP = 7,
    UNARY_OPERATION_LOG = 8,
    UNARY_OPERATION_SIN = 9,
    UNARY_OPERATION_COS = 10,
    UNARY_OPERATION_TAN = 11,
    UNARY_OPERATION_ASIN = 12,
    UNARY_OPERATION_ACOS = 13,
    UNARY_OPERATION_ATAN = 14,
    UNARY_OPERATION_RECIPROCAL = 15,
    UNARY_OPERATION_TANH = 16
} unary_type_t;

typedef struct unary_config {
    /// operation type
    operation_type_t type;

    /// operation
    unary_type_t unary_type;
}unary_config_t;

typedef enum binary_operation_type {
    BINARY_OPERATION_ADD = 0,
    BINARY_OPERATION_SUB = 1,
    BINARY_OPERATION_MUL = 2,
    BINARY_OPERATION_DIV = 3,
    BINARY_OPERATION_MAX = 4,
    BINARY_OPERATION_MIN = 5,
    BINARY_OPERATION_POW = 6,
} binary_operation_type_t;

typedef struct binary_config {
    /// operation type
    operation_type_t type;

    /// operation
    binary_operation_type_t binary_type;

    /// scalar
    int with_scalar;
    float b;
} binary_config_t;

/// upsample type
typedef enum upsample_type
{
    /// Use nearest neighbor interpolation as the upsample method
    neareast_neighbor_upsample_type = 0,
    /// Use nearest bilinear interpolation as the upsample method
    bilinear_upsample_type,
} upsample_type_t;

/// upsample
typedef struct upsample_config {
    /// operation type
    operation_type_t type;

    /// upsample type
    upsample_type_t upsample_type;

    /// width scale
    float width_scale;

    /// height scale
    float height_scale;
} upsample_config_t;

/**
 * prelu
 */
typedef struct prelu_config {
    /// operation type
    operation_type_t type;

    /// slope number
    int num_slope;

    /// requantize
    fixed_mul_t requantize;
} prelu_config_t;

/**
 * memory_data
 */
typedef struct memory_data_config{
    /// operation type
    operation_type_t type;
}memory_data_config_t;

/**
 * shuffle_channel
 */
typedef struct shuffle_channel_config{
    /// operation type
    operation_type_t type;
    /// group
    int group;
}shuffle_channel_config_t;

#endif //CNET_LAYER_CONFIG_H
