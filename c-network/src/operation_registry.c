/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "operation.h"

const char* operation_names[] = {
	"convolution",
	"pooling",
	"padding",
	"crop",
	"inner_product",
	"activation",
	"batch_norm",
	"slice",
	"concat",
	"reshape",
	"sigmoid",
	"softmax",
	"permute",
	"unary",
	"binary",
	"upsample",
	"prelu",
	"memory_data",
	"shuffle_channel",
};

/// Creator for convolution
DEFINE_OPERATION_CREATOR(convolution);

/// Creator for pooling
DEFINE_OPERATION_CREATOR(pooling);

/// Creator for padding
DEFINE_OPERATION_CREATOR(padding);

/// Creator for crop
DEFINE_OPERATION_CREATOR(crop);

/// Creator for inner_product
DEFINE_OPERATION_CREATOR(inner_product);

/// Creator for activation
DEFINE_OPERATION_CREATOR(activation);

/// Creator for batch_norm
DEFINE_OPERATION_CREATOR(batch_norm);

/// Creator for slice
DEFINE_OPERATION_CREATOR(slice);

/// Creator for concat
DEFINE_OPERATION_CREATOR(concat);

/// Creator for reshape
DEFINE_OPERATION_CREATOR(reshape);

/// Creator for sigmoid
DEFINE_OPERATION_CREATOR(sigmoid);

/// Creator for softmax
DEFINE_OPERATION_CREATOR(softmax);

/// Creator for permute
DEFINE_OPERATION_CREATOR(permute);

/// Creator for unary
DEFINE_OPERATION_CREATOR(unary);

/// Creator for binary
DEFINE_OPERATION_CREATOR(binary);

/// Creator for upsample
DEFINE_OPERATION_CREATOR(upsample);

/// Creator for prelu
DEFINE_OPERATION_CREATOR(prelu);

/// Creator for memory_data
DEFINE_OPERATION_CREATOR(memory_data);

/// Creator for shuffle_channel
DEFINE_OPERATION_CREATOR(shuffle_channel);



const operation_registry_entry_t operation_registry[] = {
	convolution_operation_creator, // convolution
	pooling_operation_creator, // pooling
	padding_operation_creator, // padding
	crop_operation_creator, // crop
	inner_product_operation_creator, // inner_product
	activation_operation_creator, // activation
	batch_norm_operation_creator, // batch_norm
	slice_operation_creator, // slice
	concat_operation_creator, // concat
	reshape_operation_creator, // reshape
	sigmoid_operation_creator, // sigmoid
	softmax_operation_creator, // softmax
	permute_operation_creator, // permute
	unary_operation_creator, // unary
	binary_operation_creator, // binary
	upsample_operation_creator, // upsample
	prelu_operation_creator, // prelu
	memory_data_operation_creator, // memory_data
	shuffle_channel_operation_creator, // shuffle_channel
};

const int operation_registry_entry_count = sizeof(operation_registry) / sizeof(operation_registry_entry_t);

operation_t *create_operation(int operation_type) {
    if (operation_type < 0 || operation_type >= operation_registry_entry_count){
        return NULL;
    }

    operation_creator_func operation_creator = operation_registry[operation_type];
    if (!operation_creator){
        return NULL;
    }

    operation_t *operation = operation_creator();
    return operation;
}
