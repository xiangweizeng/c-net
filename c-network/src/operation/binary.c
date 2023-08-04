
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"

#define ADD(x, y)       ((x) + (y))
#define SUB(x, y)       ((x) - (y))
#define MUL(x, y)       ((x) * (y))
#define DIV(x, y)       ((x) / (y))
#define MAX(x, y)       ((x) > (y) ? (x) : (y))
#define MIN(x, y)       ((x) < (y) ? (x) : (y))
#define POW(x, y)       (powf(x, y))
#define RSUB(x, y)      ((y) - (x))
#define RDIV(x, y)      ((y) / (x))
#define RPOW(x, y)      (powf(y, x))
#define ATAN2(x, y)     (atan2f(x, y))
#define RATAN2(x, y)    (atan2f(y, x))

FUNCTION_IRAM static inline tensor_shape_t binary_get_output_shape(tensor_shape_t a, tensor_shape_t b) {
    tensor_shape_t shape;

    shape.dims = MAX(a.dims, b.dims);
    shape.shape[0] = MAX(a.shape[0], b.shape[0]);
    shape.shape[1] = MAX(a.shape[1], b.shape[1]);
    shape.shape[2] = MAX(a.shape[2], b.shape[2]);
    shape.shape[3] = MAX(a.shape[3], b.shape[3]);

    return shape;
}

FUNCTION_IRAM static inline int get_input_index_offset(tensor_shape_t shape, int index[]) {
    int offset[4] = {0};
    for (size_t i = 0; i < 4; i++) {
        if (index[i] < shape.shape[i])
            offset[i] = index[i];
    }
    return TENSOR_SHAPE_OFFSET(shape.shape, offset);
}

typedef struct binary_int8_run_context_t {
    float scale_a;
    float scale_b;
    float scale_c;
} binary_int8_run_context_t;

typedef void (*operation_int8_run)(
        const binary_int8_run_context_t *context,
        const int8_t *,
        const int8_t *,
        int8_t *);

#define OPERATION_INT8_FUN(opname, op)                     \
    FUNCTION_IRAM static void operator_int8__##opname(     \
        const binary_int8_run_context_t *context,          \
        const int8_t* x,                                   \
        const int8_t* y,                                   \
        int8_t* out)                                       \
    {                                                       \
        float a = (float)*(x) / context->scale_a;           \
        float b = (float)*(y) / context->scale_b;           \
        *out = float2int8(context->scale_c*(op(a, b)));    \
    }

#define OPERATION_INT8_FUN_POINTER(opname)  operator_int8__##opname

OPERATION_INT8_FUN(add, ADD)

OPERATION_INT8_FUN(sub, SUB)

OPERATION_INT8_FUN(mul, MUL)

OPERATION_INT8_FUN(div, DIV)

OPERATION_INT8_FUN(max, MAX)

OPERATION_INT8_FUN(min, MIN)

OPERATION_INT8_FUN(pow, POW)

OPERATION_INT8_FUN(rsub, RSUB)

OPERATION_INT8_FUN(rdiv, RDIV)

OPERATION_INT8_FUN(rpow, RPOW)

OPERATION_INT8_FUN(atan2, ATAN2)

OPERATION_INT8_FUN(ratan2, RATAN2)

static operation_int8_run int8_operations[] = {
        OPERATION_INT8_FUN_POINTER(add),
        OPERATION_INT8_FUN_POINTER(sub),
        OPERATION_INT8_FUN_POINTER(mul),
        OPERATION_INT8_FUN_POINTER(div),
        OPERATION_INT8_FUN_POINTER(max),
        OPERATION_INT8_FUN_POINTER(min),
        OPERATION_INT8_FUN_POINTER(pow),
        OPERATION_INT8_FUN_POINTER(rsub),
        OPERATION_INT8_FUN_POINTER(rdiv),
        OPERATION_INT8_FUN_POINTER(rpow),
        OPERATION_INT8_FUN_POINTER(atan2),
        OPERATION_INT8_FUN_POINTER(ratan2),
};

typedef struct binary_int8_context_t {
    const int8_t *a;
    const int8_t *b;
    int8_t *c;
    operation_int8_run op;
    binary_int8_run_context_t* run_context;
} binary_int8_context_t;

FUNCTION_IRAM static void binary_int8_equals_shape_thread(
        binary_int8_context_t *context,
        size_t start,
        size_t tile) {
    const int8_t *ptr = context->a + start;
    const int8_t *ptr1 = context->b + start;
    int8_t *out_ptr = context->c + start;
    for (int i = 0; i < tile; i++) {
        context->op(context->run_context, &ptr[i], &ptr1[i], &out_ptr[i]);
    }
}

FUNCTION_IRAM int binary_op_run_int8(
        tensor_t *a,
        tensor_t *b,
        tensor_t *c,
        operation_int8_run op,
        binary_int8_run_context_t *run_context,
        option_t *opt) {

    tensor_shape_t a_shape = tensor_get_shape(a);
    tensor_shape_t b_shape = tensor_get_shape(b);

    if (TENSOR_SHAPE_EQUALS(a_shape.shape, b_shape.shape)) {

        size_t size = tensor_total(a);
        int thread_number = opt->thread_number > 0 ? opt->thread_number : 1;

        size_t group_size = size / thread_number;
        group_size = group_size > 0 ? group_size : size;

        binary_int8_context_t context = {a->data, b->data, c->data, op, run_context};
        PARALLELIZE_1D_TILE_1D(binary_int8_equals_shape_thread, context, size, group_size);

        return CNET_STATUS_SUCCESS;
    } else {
        tensor_shape_t out_shape = binary_get_output_shape(a_shape, b_shape);
        const int8_t *a_data = a->data;
        const int8_t *b_data = b->data;
        int8_t *c_data = c->data;

        for (int d0 = 0; d0 < out_shape.shape[0]; d0++) {
            for (int d1 = 0; d1 < out_shape.shape[1]; d1++) {
                for (int d2 = 0; d2 < out_shape.shape[2]; d2++) {
                    for (int d3 = 0; d3 < out_shape.shape[3]; d3++) {
                        int index[] = {d0, d1, d2, d3};
                        int offset_a = get_input_index_offset(a_shape, index);
                        int offset_b = get_input_index_offset(b_shape, index);
                        int offset_c = TENSOR_SHAPE_OFFSET(out_shape.shape, index);

                        op(run_context, a_data + offset_a, b_data + offset_b, c_data + offset_c);
                    }
                }
            }
        }
    }

    return CNET_STATUS_SUCCESS;
}

typedef struct binary_inplace_int8_t {
    int8_t *a;
    int8_t b;
    operation_int8_run op;
    binary_int8_run_context_t * run_context;
} binary_inplace_int8_t;

FUNCTION_IRAM static void binary_inplace_int8_thread(
        binary_inplace_int8_t *context,
        size_t start,
        size_t tile) {

    int8_t *ptr = context->a + start;
    for (int i = 0; i < tile; i++) {
        context->op(context->run_context, &ptr[i], &context->b, &ptr[i]);
    }
}

FUNCTION_IRAM static int binary_op_scalar_inplace_int8(
        tensor_t *a,
        int8_t b,
        operation_int8_run op,
        binary_int8_run_context_t *run_context,
        option_t *opt) {

    int size = tensor_total(a);
    int thread_number = opt->thread_number > 0 ? opt->thread_number : 1;

    size_t group_size = size / thread_number;
    group_size = group_size > 0 ? group_size : size;

    binary_inplace_int8_t context = {a->data, b, op, run_context};
    PARALLELIZE_1D_TILE_1D(binary_inplace_int8_thread, context, size, group_size);

    return CNET_STATUS_SUCCESS;
}

FUNCTION_IRAM static int binary_op_forward_inplace(
        binary_t *binary,
        blob_container_t *bottom,
        blob_container_t *top,
        option_t *opt) {

    operation_int8_run op = int8_operations[binary->config.binary_type];
    tensor_t *top_tensor = &top->data;

    binary_int8_run_context_t run_context = {bottom->blob->scale, 1.f / binary->config.b, top->blob->scale};
    return binary_op_scalar_inplace_int8(top_tensor, 1, op, &run_context, opt);
}


FUNCTION_IRAM static int binary_op_forward_impl(
        binary_t *binary,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt){

    blob_container_t *bottom0 = &bottom_tensors->data[0];
    blob_container_t *bottom1 = &bottom_tensors->data[1];
    blob_container_t *top = &top_tensors->data[0];

    operation_int8_run op = int8_operations[binary->config.binary_type];
    binary_int8_run_context_t run_context = {
            bottom0->blob->scale,
            bottom1->blob->scale,
            top->blob->scale
    };

    return binary_op_run_int8(&bottom0->data, &bottom1->data, &top->data, op, &run_context, opt);
}


FUNCTION_IRAM static int binary_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    binary_t *binary = (binary_t *) operation->base;
    if(binary->config.with_scalar){
        binary_op_forward_inplace(binary, bottom_tensors->data, top_tensors->data, opt);
    } else{
        binary_op_forward_impl(binary, bottom_tensors, top_tensors, opt);
    }

    return CNET_STATUS_SUCCESS;
}

FUNCTION_IRAM static int binary_setup(
        operation_t *operation,
        network_t* network,
        option_t *opt) {
    binary_t *binary = (binary_t *) operation->base;
    operation->support_inplace = binary->config.with_scalar;
    return CNET_STATUS_SUCCESS;
}

IMPL_OPERATION_CREATOR(binary) {
    operation_t *binary = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == binary) {
        printf("malloc for binary failed\n");
        return NULL;
    }

    operation_basic_info_setup(binary);
    binary->forward = binary_forward;
    binary->setup = binary_setup;
    return (operation_ptr) binary;
}
