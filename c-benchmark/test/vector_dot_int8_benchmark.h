/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef FACE_FERIFY_VECTOR_DOT_INT8_BENCHMARK_H
#define FACE_FERIFY_VECTOR_DOT_INT8_BENCHMARK_H

#include "benchmark_function.h"

typedef struct vector_add_create_args_t{
    int vector_len;
    int8_t *a;
    int8_t *b;
}vector_add_create_args_t;

BENCHMARK_DECLARE(compute_vector_dot, vector_add_create_args_t)

void benchmark_vector_dot();

#endif //FACE_FERIFY_VECTOR_DOT_INT8_BENCHMARK_H
