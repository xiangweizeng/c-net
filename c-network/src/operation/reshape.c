
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"

FUNCTION_IRAM static int reshape_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    return CNET_STATUS_SUCCESS;
}

IMPL_OPERATION_CREATOR(reshape) {
    operation_t *reshape = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == reshape) {
        printf("malloc for reshape failed\n");
        return NULL;
    }

    operation_basic_info_setup(reshape);
    reshape->forward = reshape_forward;
    reshape->support_inplace = 1;
    return (operation_ptr) reshape;
}