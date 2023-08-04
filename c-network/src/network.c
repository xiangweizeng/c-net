/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <stdio.h>
#include "network.h"
#include "operation_benchmark.h"


void network_teardown_range(network_t *network, int upper, option_t *opt) {

    for(int i = 0; i < upper; i++){
        operation_t* sf = network->run_operations[i];
        sf->teardown(sf, network, opt);
        sf->free_operation(sf);
        network->run_operations[i] = NULL;
    }

    fast_free(network->run_operations);
    network->run_operations = NULL;
}

status_t network_setup_run_operations(network_t *network, option_t *opt){
    if(network->run_operations != NULL){
        return CNET_STATUS_SUCCESS;
    }

    network->run_operations = fast_malloc(sizeof(operation_ptr) * network->operation_size);
    if(network->run_operations == NULL){
        return CNET_MEMORY_ALLOCATION_FAILED;
    }

    for(int i = 0; i < network->operation_size; i ++){
        operation_type_t *op = network->operations[i];
        operation_t* run = create_operation(*op);
        if(NULL == run){
            network_teardown_range(network, i, opt);
            return CNET_STATUS_FAILED;
        }

        run->base = op;
        if(run->setup && run->setup(run, network, opt) != CNET_STATUS_SUCCESS){
            fast_free(run);

            network_teardown_range(network, i, opt);
            return CNET_STATUS_FAILED;
        }

        network->run_operations[i] = run;
    }

    return CNET_STATUS_SUCCESS;
}

status_t network_teardown_run_operations(network_t *network, option_t *opt){
    network_teardown_range(network, network->operation_size, opt);
    return CNET_STATUS_SUCCESS;
}

const blob_info_t *network_get_input_blob_info(network_t *network, operation_t *op, int index){
    if(NULL == network || NULL == op || index < 0){
        return NULL;
    }

    for(int i = 0; i < network->operation_size; i++){
        if(network->operations[i] == op->base){
            if(network->operation_blobs[i].input_count > index){
                int blob = network->operation_blobs[i].input_blobs[index];
                return &network->blobs[blob];
            }
        }
    }
    return NULL;
}

const blob_info_t *network_get_output_blob_info(network_t *network, operation_t *op, int index){
    if(NULL == network || NULL == op || index < 0){
        return NULL;
    }

    for(int i = 0; i < network->operation_size; i++){
        if(network->operations[i] == op->base){
            if(network->operation_blobs[i].output_count > index){
                int blob = network->operation_blobs[i].output_blobs[index];
                return &network->blobs[blob];
            }
        }
    }
    return NULL;
}

session_t session_create(network_t* network) {
    session_t context = {
            .network = network,
            .blob_data = (blob_container_t*)malloc(sizeof(blob_container_t) * network->blob_size),
            .option = option_get_instance()
    };

    for (int i = 0; i < network->blob_size; i++) {
        context.blob_data[i].data = tensor_create_default();
        context.blob_data[i].blob = &network->blobs[i];
    }

    return context;
}

void session_release(session_t* context) {
    if (NULL == context || context->blob_data == NULL) {
        return;
    }

    for (int i = 0; i < context->network->blob_size; i++) {
        tensor_release(&context->blob_data[i].data);
        context->blob_data[i].data = tensor_create_default();
    }

    free(context->blob_data);
    context->blob_data = NULL;
}

void session_reset(session_t* context) {
    if (NULL == context || context->blob_data == NULL) {
        return;
    }

    for (int i = 0; i < context->network->blob_size; i++) {
        tensor_release(&context->blob_data[i].data);
    }
}


void session_set_input(session_t* context, int blob_index, tensor_t input) {
    if (NULL == context || context->blob_data == NULL) {
        return;
    }

    tensor_release(&context->blob_data[blob_index].data);
    if(input.data_type != TENSOR_DATA_INT8){
        blob_info_t output_blob = context->network->blobs[blob_index];
        int* out_shape = output_blob.shape;

        tensor_t quantize = tensor_create_default();
        tensor_create_3d(&quantize, out_shape[3], out_shape[2], out_shape[1], 1u, context->option.tensor);
        tensor_set_data_type(&quantize, TENSOR_DATA_INT8);

        quantize_tensor(&input, &quantize, output_blob.scale, &context->option);
        context->blob_data[blob_index].data = quantize;
    }else{

        tensor_add_ref(&input);
        context->blob_data[blob_index].data = input;
    }
}

tensor_t session_get_output(session_t* context, int blob_index) {
    if (NULL == context || context->blob_data == NULL) {
        return tensor_create_default();
    }

    if(context->blob_data[blob_index].data.data_type != TENSOR_DATA_FLOAT){

        blob_info_t output_blob = context->network->blobs[blob_index];
        int* out_shape = output_blob.shape;

        tensor_t output = tensor_create_default();
        tensor_create_3d(&output, out_shape[3], out_shape[2], out_shape[1], 4u, context->option.tensor);
        tensor_set_data_type(&output, TENSOR_DATA_FLOAT);

        quantize_tensor(&context->blob_data[blob_index].data, &output, output_blob.scale, &context->option);
        tensor_release(&context->blob_data[blob_index].data);
        context->blob_data[blob_index].data = output;
    }

    tensor_add_ref(&context->blob_data[blob_index].data);
    return context->blob_data[blob_index].data;
}

FUNCTION_IRAM static void vector_blob_container_release(vector_blob_container_t *blobs) {
    for(int i = 0; i < (*blobs).current_size; i ++){
        (*blobs).data[i].blob = NULL;
        tensor_release(&(*blobs).data[i].data);
    }
    (*blobs).free_data(blobs);
}

FUNCTION_IRAM static int session_forward_operation(
        session_t *context,
        operation_t *operation,
        operation_blobs_t *op_blob,
        option_t *opt) {

    vector_blob_container_t bottom_blobs = VECTOR_CREATE(blob_container_t, op_blob->input_count);
    for (size_t i = 0; i < op_blob->input_count; i++) {

        int bottom_blob_index = op_blob->input_blobs[i];
        if (context->blob_data[bottom_blob_index].data.dims == 0) {
            printf("operation %s network_forward, can not run, tensor index: %d is empty\n",
                   operation_names[*operation->base],
                   bottom_blob_index
                   );

            vector_blob_container_release(&bottom_blobs);
            return CNET_STATUS_FAILED;
        }

        bottom_blobs.push_element(&bottom_blobs, &context->blob_data[bottom_blob_index]);
        tensor_add_ref(&bottom_blobs.data[i].data);

        if (opt->light_mode) {

            if(NULL != bottom_blobs.data[i].data.refcount && *bottom_blobs.data[i].data.refcount == 2){
                /// delete after taken in light mode
                tensor_release(&context->blob_data[bottom_blob_index].data);
            } else {
                /// to release
                tensor_t to_release = bottom_blobs.data[i].data;
                tensor_release(&to_release);

                /// deep copy for inplace forward if data is shared
                if (operation->support_inplace) {

                    /// add ref should to release
                    tensor_release(&bottom_blobs.data[i].data);
                    bottom_blobs.data[i].data = tensor_clone(&context->blob_data[bottom_blob_index].data);
                }
            }
        }
    }

    vector_blob_container_t top_blobs = VECTOR_CREATE(blob_container_t, op_blob->output_count);
    if (operation->support_inplace) {

        /// prepare tensor
        for (size_t i = 0; i < op_blob->output_count; i++) {
            int top_blob_index = op_blob->output_blobs[i];
            const blob_info_t *info = context->blob_data[top_blob_index].blob;

            tensor_t temp = tensor_reshape_3d(
                    &bottom_blobs.data[i].data,
                    info->shape[3],
                    info->shape[2],
                    info->shape[1]);

            if(!context->option.light_mode){
                tensor_t to_release = temp;
                tensor_release(&to_release);
                temp = tensor_clone(&temp);
            }

            blob_container_t blob = {
                    .data = temp,
                    .blob = info,
            };
            top_blobs.push_element(&top_blobs, &blob);
        }
    } else
    {
        /// prepare tensor
        for (size_t i = 0; i < op_blob->output_count; i++) {
            int top_blob_index = op_blob->output_blobs[i];
            const blob_info_t *info = context->blob_data[top_blob_index].blob;

            tensor_data_type_t type = TENSOR_DATA_INT16;
            size_t elem_size = 2;
            if(info->data_type ==  float_data_type){
                type = TENSOR_DATA_FLOAT;
                elem_size = 4;
            }

            blob_container_t blob = {
                    .data = tensor_create_default(),
                    .blob = info,
            };

            tensor_create_3d(&blob.data, info->shape[3], info->shape[2], info->shape[1], elem_size, opt->tensor);
            tensor_set_data_type(&blob.data, type);
            top_blobs.push_element(&top_blobs, &blob);
        }
    }

    /// forward operation
    int ret = operation->forward(operation, &bottom_blobs, &top_blobs, opt);
    if (CNET_STATUS_SUCCESS != ret){
        vector_blob_container_release(&bottom_blobs);
        vector_blob_container_release(&top_blobs);
        return ret;
    }

    /// store top blobs
    for (size_t i = 0; i < op_blob->output_count; i++) {
        /// store
        int top_blob_index = op_blob->output_blobs[i];
        if(context->blob_data[top_blob_index].data.dims == 0){
            context->blob_data[top_blob_index].data = top_blobs.data[i].data;
            const blob_info_t *info = context->blob_data[top_blob_index].blob;

            /// deal consumers_count
            if(opt->light_mode && info->consumers_count > 0){
                for(int s = 0; s < info->consumers_count; s++){
                    tensor_add_ref(&context->blob_data[top_blob_index].data);
                }
            } else
            {
                tensor_add_ref(&context->blob_data[top_blob_index].data);
            }
        }
    }

    /// release
    vector_blob_container_release(&bottom_blobs);
    vector_blob_container_release(&top_blobs);
    return CNET_STATUS_SUCCESS;
}


status_t session_run(session_t* context) {
    if (NULL == context || context->blob_data == NULL) {
        return CNET_STATUS_FAILED;
    }

    if(context->network->run_operations == NULL){
        return CNET_STATUS_FAILED;
    }

    clear_spend_time();
    TIME_MARK_BEGIN(session_run)

    network_t* network = context->network;
    for (int i = 0; i < network->operation_size; i++) {
        operation_t * op = network->run_operations[i];
        operation_blobs_t *op_blob = &network->operation_blobs[i];


        TIME_MARK_BEGIN(session_forward_operation)
        status_t ret = session_forward_operation(context, op, op_blob, &context->option);
        operation_spend_time(*op->base, TIME_MARK_END(session_forward_operation));

        if (ret != CNET_STATUS_SUCCESS) {
            printf("run operation error: %d, %d, %d\n", i, *op->base, ret);
            return ret;
        }
    }

    TIME_MARK_PRINT(session_run)
    print_spend_times();

    return CNET_STATUS_SUCCESS;
}

void session_tensor_status(const session_t *session) {

    printf("session tensor:\n");
    for(int c = 0; c < session->network->blob_size; c ++){
//        blob_info_t *info = &context->network->blobs[c];
        if(session->blob_data[c].data.refcount){
            printf("%d:%d ", c, *session->blob_data[c].data.refcount);
        }
    }
    printf("\n");
}
