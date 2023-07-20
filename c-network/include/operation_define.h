/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_LAYER_DEFINE_H
#define CNET_LAYER_DEFINE_H

#include "operation.h"
#include "tensor.h"

#ifndef NULL
#define NULL
#endif

#define BLOB_INSTANCE(n, c, h, w, dt, used, sc)         \
            {                                           \
                .shape = {n, c, h, w},                  \
                .data_type = (data_type_t)dt,           \
                .consumers_count = used,                \
                .scale = sc,                            \
            }

#define REFERENCE_LAYER(operation, name)                \
        &(layer_##name##_##operation.config.type)

#define DEFINE_TENSOR(name, size, tensor_data, sc)      \
static const weight_container_t name = {                \
   .data = {                                            \
        .data = (void*)tensor_data,                     \
        .elem_size = 2,                                 \
        .data_type = TENSOR_DATA_INT16,                 \
        .dims = 1,                                      \
        .d3 = 1,                                        \
        .d2 = 1,                                        \
        .d1 = 1,                                        \
        .d0 = size,                                     \
        .step = size,                                   \
        .refcount = NULL,                               \
        .layout = TENSOR_LAYOUT_NCHW,                   \
        .allocator = NULL,                              \
    },                                                  \
    .scale = sc,                                        \
};

#define DEFINE_CONVOLUTION_LAYER(name,                  \
                    batch, channel,                     \
                    height, width,                      \
                    dp, s_w, s_h,                       \
                    d_w, d_h,                           \
                    pl, pr, pt, pb,                     \
                    pv, bs,                             \
                    rq_m, rq_s,                         \
                    lk_m, lk_s,                         \
                    max_, min_)                         \
static convolution_t layer_##name##_convolution = {     \
        .config = {                                     \
            .type = convolution_operation_type,         \
            .filers_size = {batch,channel,height,width},\
            .group = dp,                                \
            .stride_w = s_w,                            \
            .stride_h = s_h,                            \
            .dilation_w = d_w,                          \
            .dilation_h = d_h,                          \
            .pad_left = pl,                             \
            .pad_right = pr,                            \
            .pad_top = pt,                              \
            .pad_bottom = pb,                           \
            .pad_value = pv,                            \
            .bias_term = bs,                            \
            .requantize = {rq_m, rq_s},                 \
            .leaky = {lk_m, lk_s},                      \
            .max = max_,                                \
            .min = min_,                                \
        },                                              \
        .filters = layer_##name##_convolution_filters,  \
        .bias = layer_##name##_convolution_bias,        \
};

#define DEFINE_POOLING_LAYER(name, p_t,                 \
                                k_w, k_h,               \
                                s_w ,s_h,               \
                                g_b,                    \
                                pl, pr, pt, pb,         \
                                pv,                     \
                                c_p, rq_m, rq_s)        \
static pooling_t layer_##name##_pooling = {             \
        .config = {                                     \
            .type = pooling_operation_type,             \
            .pooling_type = (pool_method_type_t)p_t,    \
            .kernel_w = k_w,                            \
            .kernel_h = k_h,                            \
            .stride_w = s_w,                            \
            .stride_h = s_h,                            \
            .global_pooling = g_b,                      \
            .pad_left = pl,                             \
            .pad_right = pr,                            \
            .pad_top = pt,                              \
            .pad_bottom = pb,                           \
            .pad_value = pv,                            \
            .count_include_pad = c_p,                   \
            .requantize = {rq_m, rq_s},                 \
        },                                              \
};

#define DEFINE_PADDING_LAYER(name, p_m,                 \
                        pl, pr, pt, pb, pv)             \
static padding_t layer_##name##_padding = {             \
        .config = {                                     \
            .type = pooling_operation_type,             \
            .pad_mode = (padding_mode_t)p_m,            \
            .pad_left = pl,                             \
            .pad_right = pr,                            \
            .pad_top = pt,                              \
            .pad_bottom = pb,                           \
            .pad_value = pv,                            \
        },                                              \
};

#define DEFINE_CROP_LAYER(name, d_m, w_o ,h_o, c_o,     \
                        o_w, o_h, o_c,                  \
                        rq_m, rq_s)                     \
static crop_t layer_##name##_crop = {                   \
        .config = {                                     \
            .type = crop_operation_type,                \
            .dims = d_m,                                \
            .w_offset = w_o,                            \
            .h_offset = h_o,                            \
            .c_offset = c_o,                            \
            .out_w = o_w,                               \
            .out_h = o_h,                               \
            .out_c = o_c,                               \
            .requantize = {rq_m, rq_s},                 \
        },                                              \
};

#define DEFINE_INNER_PRODUCT_LAYER(name,                \
                    batch, channel,                     \
                    height, width,                      \
                    bs,                                 \
                    rq_m, rq_s,                         \
                    lk_m, lk_s,                         \
                    max_, min_)                         \
static inner_product_t layer_##name##_inner_product = { \
        .config = {                                     \
            .type = inner_product_operation_type,       \
            .filers_size = {batch,channel,height,width},\
            .bias_term = bs,                            \
            .requantize = {rq_m, rq_s},                 \
            .leaky = {lk_m, lk_s},                      \
            .max = max_,                                \
            .min = min_,                                \
        },                                              \
        .weights = layer_##name##_inner_product_filters,\
        .bias = layer_##name##_inner_product_bias,      \
};

#define DEFINE_ACTIVATION_LAYER(name, a_t, ...)         \
static activation_t layer_##name##_activation = {       \
        .config ={                                      \
            .type = activation_operation_type,          \
            .activate_type = (activate_type_t)a_t,      \
            .activate_params = {__VA_ARGS__},           \
        }                                               \
};

#define DEFINE_BATCH_NORM_LAYER(name, rq_m, rq_s)       \
static batch_norm_t layer_##name##_batch_norm = {       \
        .config ={                                      \
            .type = batch_norm_operation_type,          \
            .requantize = {rq_m, rq_s},                 \
        },                                              \
        .scale = layer_##name##_batch_norm_scale,       \
        .offset = layer_##name##_batch_norm_offset,     \
};

#define DEFINE_SLICE_LAYER(name, a_x, rq_m, rq_s)       \
static slice_t layer_##name##_slice = {                 \
        .config ={                                      \
            .type = slice_operation_type,               \
            .axis = a_x,                                \
            .requantize = {rq_m, rq_s},                 \
        },                                              \
        .slices = layer_##name##_slice_slices,          \
};

#define DEFINE_CONCAT_LAYER(name, a_x, i_c, ...)        \
static concat_t layer_##name##_concat = {               \
        .config ={                                      \
            .type = concat_operation_type,              \
            .axis = a_x,                                \
            .concat_count = i_c,                        \
            .output_fixed_mul = {__VA_ARGS__},          \
        }                                               \
};

#define DEFINE_RESHAPE_LAYER(name)                      \
static reshape_t layer_##name##_reshape = {             \
        .config ={                                      \
            .type = reshape_operation_type,             \
        }                                               \
};

#define DEFINE_SIGMOID_LAYER(name)                      \
static sigmoid_t layer_##name##_sigmoid = {             \
        .config ={                                      \
            .type = sigmoid_operation_type,             \
        }                                               \
};

#define DEFINE_SOFTMAX_LAYER(name, axis_index)          \
static softmax_t layer_##name##_softmax = {             \
        .config ={                                      \
            .type = softmax_operation_type,             \
            .axis = axis_index,                         \
        }                                               \
};

#define PERMUTE_ORDER0      { 0, 1, 2, 3 }
#define PERMUTE_ORDER1      { 0, 1, 3, 2 }
#define PERMUTE_ORDER2      { 0, 1, 2, 3 }
#define PERMUTE_ORDER3      { 0, 1, 3, 2 }
#define PERMUTE_ORDER4      { 0, 2, 1, 3 }
#define PERMUTE_ORDER5      { 0, 2, 3, 1 }
#define PERMUTE_ORDER6      { 0, 3, 1, 2 }
#define PERMUTE_ORDER7      { 0, 3, 2, 1 }
#define DEFINE_PERMUTE_LAYER(name, axis_index)          \
static permute_t layer_##name##_permute = {             \
        .config ={                                      \
            .type = permute_operation_type,             \
            .axis = PERMUTE_ORDER##axis_index,          \
        }                                               \
};

#define DEFINE_UNARY_LAYER(name, u_t)                   \
static unary_t layer_##name##_unary = {                 \
        .config ={                                      \
            .type = unary_operation_type,               \
            .unary_type = (unary_type_t)u_t,            \
        }                                               \
};

#define DEFINE_BINARY_LAYER(name, b_t, w_s, w_b)        \
static binary_t layer_##name##_binary = {               \
        .config ={                                      \
            .type = binary_operation_type,              \
            .binary_type = (binary_operation_type_t)b_t,\
            .with_scalar = w_s,                         \
            .b = w_b,                                   \
        }                                               \
};

#define DEFINE_UP_SAMPLE_LAYER(name, u_t, w_s, h_s)     \
static upsample_t layer_##name##_upsample = {           \
        .config ={                                      \
            .type = upsample_operation_type,            \
            .upsample_type = (upsample_type_t)u_t,      \
            .width_scale = w_s,                         \
            .height_scale = h_s,                        \
        }                                               \
};

#define DEFINE_PRELU_LAYER(name, s_n, rq_m, rq_s)       \
static prelu_t layer_##name##_prelu = {                 \
        .config = {                                     \
            .type = prelu_operation_type,               \
            .num_slope = s_n,                           \
            .requantize = {rq_m, rq_s},                 \
        },                                              \
        .slope = layer_##name##_prelu_slope,            \
};

#define DEFINE_MEMORY_DATA_LAYER(name)                  \
static memory_data_t layer_##name##_memory_data = {     \
        .config = {                                     \
            .type = memory_data_operation_type,         \
        },                                              \
        .data = layer_##name##_shuffle_channel_data,    \
};

#define DEFINE_SHUFFLE_CHANNEL_LAYER(name, c_g)         \
static shuffle_channel_t layer_##name##_shuffle_channel \
        = {                                             \
        .config = {                                     \
            .type = shuffle_channel_operation_type,     \
            .group = c_g,                               \
        },                                              \
};

#endif //CNET_LAYER_DEFINE_H
