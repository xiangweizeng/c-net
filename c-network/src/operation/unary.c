
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"

#define ABS(x) (fabs(x))
#define NEG(x) (-(x))
#define FLOOR(x) (floor(x))
#define CEIL(x) (ceil(x))
#define SQUARE(x) ((x) * (x))
#define SQRT(x) (sqrt(x))
#define RSQRT(x) (1.f / sqrt(x))
#define EXP(x) (exp(x))
#define LOG(x) (log(x))
#define SIN(x) (sin(x))
#define COS(x) (cos(x))
#define TAN(x) (tan(x))
#define ASIN(x) (asin(x))
#define ACOS(x) (acos(x))
#define ATAN(x) (atan(x))
#define REC(x) (1.f / (x))
#define TANH(x) (tanh(x))

typedef struct binary_int16_run_context_t {
    float scale_a;
    float scale_b;
} binary_int16_run_context_t;

typedef void (*operation_run)(const binary_int16_run_context_t *, int16_t *);

#define OPERATION_FUN(opname, op)                           \
    FUNCTION_IRAM static void operator_int16__##opname(     \
        const binary_int16_run_context_t *context,          \
        int16_t* x)                                         \
    {                                                       \
        float a = (float)*(x) / context->scale_a;           \
        *(x) = float2int16(context->scale_b * (op(a)));         \
    }

#define OPERATION_FUN_POINTER(opname) operator_int16__##opname

OPERATION_FUN(abs, ABS)

OPERATION_FUN(neg, NEG)

OPERATION_FUN(floor, FLOOR)

OPERATION_FUN(ceil, CEIL)

OPERATION_FUN(square, SQUARE)

OPERATION_FUN(sqrt, SQRT)

OPERATION_FUN(rsqrt, RSQRT)

OPERATION_FUN(exp, EXP)

OPERATION_FUN(log, LOG)

OPERATION_FUN(sin, SIN)

OPERATION_FUN(cos, COS)

OPERATION_FUN(tan, TAN)

OPERATION_FUN(asin, ASIN)

OPERATION_FUN(acos, ACOS)

OPERATION_FUN(atan, ATAN)

OPERATION_FUN(rec, REC)

OPERATION_FUN(tanh, TANH)

operation_run operation_table[] = {
        OPERATION_FUN_POINTER(abs),
        OPERATION_FUN_POINTER(neg),
        OPERATION_FUN_POINTER(floor),
        OPERATION_FUN_POINTER(ceil),
        OPERATION_FUN_POINTER(square),
        OPERATION_FUN_POINTER(sqrt),
        OPERATION_FUN_POINTER(rsqrt),
        OPERATION_FUN_POINTER(exp),
        OPERATION_FUN_POINTER(log),
        OPERATION_FUN_POINTER(sin),
        OPERATION_FUN_POINTER(cos),
        OPERATION_FUN_POINTER(tan),
        OPERATION_FUN_POINTER(asin),
        OPERATION_FUN_POINTER(acos),
        OPERATION_FUN_POINTER(atan),
        OPERATION_FUN_POINTER(rec),
        OPERATION_FUN_POINTER(tanh)
};

typedef struct unary_op_context {
    binary_int16_run_context_t *run_context;
    void *data;
    operation_run op;
} unary_op_context;

FUNCTION_IRAM static void unary_op_thread_tile(unary_op_context *context, size_t index, size_t tile) {
    int16_t *a = (int16_t *) context->data + index;
    for (int i = 0; i < tile; i++) {
        context->op(context->run_context, a + i);
    }
}

FUNCTION_IRAM static int unary_op_inplace(
        tensor_t *a,
        binary_int16_run_context_t *run_context,
        operation_run op,
        option_t *opt) {
    size_t size = tensor_total(a);
    int thread_number = opt->thread_number > 0 ? opt->thread_number : 1;

    size_t group_size = size / thread_number;
    group_size = group_size > 0 ? group_size : size;

    unary_op_context context = {
            run_context,
            a->data,
            op
    };

    PARALLELIZE_1D_TILE_1D(unary_op_thread_tile, context, size, group_size);
    return CNET_STATUS_SUCCESS;
}

FUNCTION_IRAM static int unary_op_forward_inplace(
        unary_t *unary,
        blob_container_t *bottom,
        blob_container_t *top,
        option_t *opt) {

    operation_run op = operation_table[unary->config.unary_type];
    tensor_t *top_tensor = &top->data;

    binary_int16_run_context_t run_context = {
            bottom->blob->scale,
            top->blob->scale
    };

    return unary_op_inplace(top_tensor, &run_context, op, opt);
}

FUNCTION_IRAM static int unary_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    unary_t *unary = (unary_t *) operation->base;
    return unary_op_forward_inplace(unary, bottom_tensors->data, top_tensors->data, opt);
}

IMPL_OPERATION_CREATOR(unary) {
    operation_t *unary = (operation_t *) fast_malloc(sizeof(operation_t));
    if (NULL == unary) {
        printf("malloc for unary failed\n");
        return NULL;
    }

    operation_basic_info_setup(unary);
    unary->forward = unary_forward;
    return (operation_ptr) unary;
}
