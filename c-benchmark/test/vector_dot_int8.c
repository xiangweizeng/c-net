/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <stdint.h>

int32_t compute_vector_dot(int8_t *a, int8_t *b, int32_t len)
{
    int32_t sum = 0;

    for(int i = 0; i < len ; i++){
        sum += a[i]*b[i];
    }

    return sum;
}