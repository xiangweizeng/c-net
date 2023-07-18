/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_NETWORK_H
#define CNET_NETWORK_H

#include "operation.h"

/**
 * Network
 */
typedef struct network {
    /// blob size
    int blob_size;

    /// blob infos
    blob_info_t* blobs;

    /// operation info
    int operation_size;

    /// operation infos
    operation_type_t ** operations;

    /// operation blobs
    operation_blobs_t *operation_blobs;

    /// run operations
    operation_ptr *run_operations;
}network_t;

/// Setup run operations
/// \param network
/// \param opt
/// \return
status_t network_setup_run_operations(network_t *network, option_t *opt);

/// Tear down operations
/// \param network
/// \param opt
/// \return
status_t network_teardown_run_operations(network_t *network, option_t *opt);

/// Get input blob info form operation and index
/// \param network
/// \param op
/// \param index
/// \return
const blob_info_t *network_get_input_blob_info(network_t *network, operation_t *op, int index);

/// Get output blob info form operation and index
/// \param network
/// \param op
/// \param index
/// \return
const blob_info_t *network_get_output_blob_info(network_t *network, operation_t *op, int index);

/**
 * Session context
 */
typedef struct session {
    /**
     * network
     */
    network_t* network;

    /**
     * blob data
     */
    blob_container_t* blob_data;

    /**
    * option
    */
    option_t option;
}session_t;

/**
 * Create session
 * @param network
 * @return
 */
session_t session_create(network_t* network);

/**
 * Release session
 * @param context
 */
void session_release(session_t* context);

/**
 * Session set input
 * @param context
 * @param blob_index
 * @param input
 */
void session_set_input(session_t* context, int blob_index, tensor_t input);

/**
 * Sesssion get output
 * @param context
 * @param blob_index
 * @return
 */
tensor_t session_get_output(session_t* context, int blob_index);

/**
 * Session run network
 * @param context
 * @return
 */
status_t session_run(session_t* context);

/**
 * Print session status
 * @param session
 */
void session_tensor_status(const session_t *session);

#endif //CNET_NETWORK_H
