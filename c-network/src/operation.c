/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "operation.h"

VECTOR_IMPL(blob_container_t)

static int operation_setup_default(operation_t *operation, network_t* network, option_t *opt) {
    return CNET_STATUS_SUCCESS;
}

static int operation_teardown_default(operation_t *operation, network_t* network, option_t *opt) {
    return CNET_STATUS_SUCCESS;
}

static void operation_free_operation_default(operation_t *operation) {
    fast_free(operation);
}

void operation_basic_info_setup(operation_t *operation) {
    operation->support_inplace = 0;
    operation->forward = NULL;
    operation->setup = operation_setup_default;
    operation->teardown = operation_teardown_default;
    operation->free_operation = operation_free_operation_default;
}
