/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_STATUS_H
#define CNET_STATUS_H

typedef enum status_t{
    /**
     * failed
     */
    CNET_STATUS_FAILED = -1,

    /**
     * sucess
     */
    CNET_STATUS_SUCCESS  = 0,

    /**
     * memory allocation failed
     */
    CNET_MEMORY_ALLOCATION_FAILED = 1,

}status_t;

#endif //CNET_STATUS_H
