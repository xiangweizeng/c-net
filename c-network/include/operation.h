
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_LAYER_H
#define CNET_LAYER_H

#include "operation_config.h"

/// Blob container
typedef struct blob_container {
    /// blob info
    const blob_info_t *blob;

    /// data
    tensor_t data;
}blob_container_t;

/**
 * Blob container ptr
 */
typedef blob_container_t* blob_container_ptr;

/**
 * vector for blob_container_t
 */
VECTOR_DEFINE(blob_container_t)

/**
 * network
 */
struct network;
typedef struct network network_t;

/**
 * Operation
 */
struct operation_t;
typedef struct operation_t operation_t;

/**
 * operation setup interface
 */
typedef int setup_func(operation_t *, network_t*, option_t *);

/**
 * operation teardown interface
 */
typedef int teardown_func(operation_t *, network_t*, option_t *);

/**
 * operation forward for tensors interface
 */
typedef int forward_func(operation_t *, vector_blob_container_t*, vector_blob_container_t *, option_t *);

/**
 * operation free interface
 */
typedef void free_operation_func(operation_t *);

/**
 * Operation
 */
typedef struct operation_t {
    /// Base info
    operation_type_t *base;

    /// 0 yes 1 support, support inplace
    int support_inplace;

    /// setup
    setup_func *setup;

    /// teardown
    teardown_func *teardown;

    /// forward for tensors
    forward_func *forward;

    /// free operation
    free_operation_func *free_operation;
} *operation_ptr;


/// Set up operation basic info
/// \param operation
void operation_basic_info_setup(operation_t *operation);

/**
 * operation factory function
 */
typedef operation_t *(*operation_creator_func)();

/**
 *  operation factory entry
 */
typedef operation_creator_func operation_registry_entry_t;

/**
 * Create operation form operation type
 * @param operation_type
 * @return
 */
FUNCTION_IRAM operation_t *create_operation(int operation_type);

/**
 * Define operation creator
 */
#define DEFINE_OPERATION_CREATOR(name) operation_t* name##_operation_creator();

/**
 * Impl operation creator
 */
#define IMPL_OPERATION_CREATOR(name) operation_t* name##_operation_creator()


/**
* weight container
*/
typedef struct weight_container {
    /// data
    tensor_t data;

    /// scale
    float scale;
}weight_container_t;

/**
 * convolution operation
 */
typedef struct convolution {
    /// config
    convolution_config_t config;

    /// filters
    weight_container_t filters;

    /// bias
    weight_container_t bias;
} convolution_t;

/**
 * pooling operation
 */
typedef struct pooling {
    pooling_config_t config;
} pooling_t;

/**
 * padding operation
 */
typedef struct padding {
    padding_config_t config;
} padding_t;

/**
 * crop operation
 */
typedef struct crop{
    crop_config_t config;
}crop_t;

/**
 * inner product
 */
typedef struct inner_product{
    /// config
    inner_product_config_t config;

    /// weights
    weight_container_t weights;

    /// bias
    weight_container_t bias;
} inner_product_t;

/**
 * activation
 */
typedef struct activation {
    /// config
    activate_config_t config;
} activation_t;


/**
 * batch norm
 */
typedef struct batch_norm{
    /// config
    batch_norm_config_t config;

    /// scale
    weight_container_t scale;

    /// offset
    weight_container_t offset;
}batch_norm_t;

/**
 * slice
 */
typedef struct slice {
    /// config
    slice_config_t config;

    /// slices
    weight_container_t slices;
} slice_t;

/**
 * concat
 */
typedef struct concat {
    concat_config_t config;
} concat_t;

/**
 * reshape
 */
typedef struct reshape {
    reshape_config_t config;
} reshape_t;

/**
 * sigmoid
 */
typedef struct sigmoid {
    sigmoid_config_t config;
} sigmoid_t;

/**
 * softmax
 */
typedef struct softmax {
    softmax_config_t config;
} softmax_t;

/**
 * permute
 */
typedef struct permute {
    permute_config_t config;
} permute_t;

/**
 * binary
 */
typedef struct unary {
    unary_config_t config;
} unary_t;

/**
 * binary
 */
typedef struct binary {
    binary_config_t config;
} binary_t;

/**
 * upsample2x
 */
typedef struct upsample {
    upsample_config_t config;
} upsample_t;

/**
 * prelu
 */
typedef struct prelu{
    prelu_config_t config;

    /// slices
    weight_container_t slope;
}prelu_t;

/**
 * memory_data
 */
typedef struct memory_data{
    /// operation type
    memory_data_config_t config;

    /// data
    weight_container_t data;
}memory_data_t;

/**
 * shuffle_channel
 */
typedef struct shuffle_channel{
    /// config
    shuffle_channel_config_t config;
}shuffle_channel_t;


#endif //CNET_LAYER_H
