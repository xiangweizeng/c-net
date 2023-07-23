/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_TENSOR_H
#define CNET_TENSOR_H

#include <stddef.h>
#include <stdint.h>
#include <option.h>
#include "allocator.h"
#include "container_vector_define.h"

/**
 * tensor layout
 */
typedef enum tensor_layout_t{
    TENSOR_LAYOUT_NCHW = 0,             // nchw layout
    TENSOR_LAYOUT_NHWC = 1,             // nhwc layout
}tensor_layout_t;

/**
 * tensor data type
 */
typedef enum tensor_data_type_t{
    TENSOR_DATA_FLOAT = 0,      // float
    TENSOR_DATA_FLOAT16 = 1,    // float16
    TENSOR_DATA_INT32 = 3,      // int32
    TENSOR_DATA_INT16 = 4,      // int16
    TENSOR_DATA_INT8 = 5,       // int8
    TENSOR_DATA_UINT8 = 6      // uint8
}tensor_data_type_t;

typedef struct tensor_shape_t{
    /**
     * dims
     */
    size_t dims;

    /**
     * shape
     * shape[0] shape[1] shape[2] shape[3]
     */
    size_t shape[4];
}tensor_shape_t;

typedef struct tensor_t
{
    /**
     *  pointer to the data
     *  data_size = elem_size*n*c*h*d0
     */
    void *data;

    /**
     *  element size in bytes
     *  4 = float32/int32
     *  2 = float16
     *  1 = int8/uint8
     *  0 = empty
     */
    size_t elem_size;

    /**
     * data type, default float32
     */
    tensor_data_type_t data_type;

    /**
     * the dimensionality
     */
    int dims;
    int d3;
    int d2;
    int d1;
    int d0;


    /**
     * channel/d0c step
     */
    size_t step;

    /**
     * pointer to the reference counter
     * d0hen points to user-allocated data, the pointer is NULL
     */
    int *refcount;

    /**
     * tensor layout
     */
    tensor_layout_t layout;

    /**
     * pointer to the allocator
     */
    allocator_t *allocator;
} tensor_t;

/**
 * vector for tensor_t
 */
VECTOR_DEFINE(tensor_t)
FUNCTION_IRAM vector_tensor_t vector_tensor_create_default(int tensor_count);
FUNCTION_IRAM void vector_tensor_free(vector_tensor_t *tensors);

/**
 * common operator
 * 1. create default, all tensor, must init by create_default
 * 2. create like, like tensor,contains layout and cstep, but use allocator, if allocator, use default malloc and free
 * 3. refcount operator
 * 4. empty,total
 * 5. clone, shape, data, and layout, use same allocator
 * 6. fill
 * 7. set data
 * 8. set quantize param
 * 9. get tensor shape
 */
FUNCTION_IRAM tensor_t tensor_create_default();
FUNCTION_IRAM void tensor_create_like(tensor_t *tensor, const tensor_t *like, size_t element_size, allocator_t *allocator);
FUNCTION_IRAM void tensor_add_ref(tensor_t *tensor);// refcount++
FUNCTION_IRAM void tensor_release(tensor_t *tensor);// refcount--
FUNCTION_IRAM int tensor_empty(tensor_t *tensor);
FUNCTION_IRAM size_t tensor_total(tensor_t *tensor);
FUNCTION_IRAM tensor_t tensor_clone(tensor_t *tensor);
FUNCTION_IRAM void tensor_fill_f(tensor_t *tensor, float v);
FUNCTION_IRAM void tensor_fill_i32(tensor_t *tensor, int v);
FUNCTION_IRAM void tensor_fill_i8(tensor_t *tensor, char v);
FUNCTION_IRAM void tensor_fill_u8(tensor_t *tensor, unsigned char v);
FUNCTION_IRAM void tensor_set_data_type(tensor_t *tensor, tensor_data_type_t data_type);
FUNCTION_IRAM tensor_shape_t tensor_get_shape(tensor_t *tensor);
#define TENSOR_SHAPE_OFFSET(shape, index) (((index[0] * shape[1] + index[1]) * shape[2] + index[2]) * shape[3] + index[3])
#define TENSOR_SHAPE_EQUALS(a, b) (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3])

/**
 * Operator for data
 * 1. reshape
 * 2. create tensor is created, and the basic info is small, not create
 * 3. create d0ith data
 * 4. tensor data reference
 * For all create, before create, will call tensor_release
 * so tensor must init by tensor_create_default()
 */
FUNCTION_IRAM tensor_t tensor_reshape_1d(tensor_t *tensor, int d0);
FUNCTION_IRAM tensor_t tensor_reshape_2d(tensor_t *tensor, int d0, int d1);
FUNCTION_IRAM tensor_t tensor_reshape_3d(tensor_t *tensor, int d0, int d1, int d2);
FUNCTION_IRAM void tensor_create_1d(tensor_t *tensor, int d0, size_t elem_size, allocator_t *allocator);
FUNCTION_IRAM void tensor_create_2d(tensor_t *tensor, int d0, int d1, size_t elem_size, allocator_t* allocator);
FUNCTION_IRAM void tensor_create_3d(tensor_t *tensor, int d0, int d1, int d2, size_t elem_size, allocator_t *allocator);
FUNCTION_IRAM void tensor_create_1d_data(tensor_t *tensor, int d0, void *data, size_t elem_size);
FUNCTION_IRAM void tensor_create_2d_data(tensor_t *tensor, int d0, int d1, void *data, size_t elem_size);
FUNCTION_IRAM void tensor_create_3d_data(tensor_t *tensor, int d0, int d1, int d2, void *data, size_t elem_size);
FUNCTION_IRAM tensor_t tensor_d2(tensor_t *tensor, int d2);// data reference, for d2
FUNCTION_IRAM tensor_t tensor_d2_range(tensor_t *tensor, int d2, int number);// range reference, for d2
FUNCTION_IRAM tensor_t tensor_range(tensor_t *tensor, int x, int n);// data reference x for d0
FUNCTION_IRAM tensor_t tensor_from_float16(const unsigned short *data, int size, allocator_t *allocator);

/**
 * Print tensor
 */
#define PRINT_DATA_DECLARE(T) void print_data_##T(T*data, size_t d0, size_t d1, size_t d2)
PRINT_DATA_DECLARE(int8_t);
PRINT_DATA_DECLARE(uint8_t);
PRINT_DATA_DECLARE(int64_t);
PRINT_DATA_DECLARE(float);

/// Print tensor for float
/// \param tensor
void tensor_print(tensor_t *tensor);

/// Print tensor for uint8
/// \param tensor
void tensor_print_u8(tensor_t *tensor);

/// Print tensor for int32
/// \param tensor
void tensor_print_int32(tensor_t *tensor);

/// Tensor from chw to hwc
/// \param chw
/// \param opt
/// \return
FUNCTION_IRAM tensor_t tensor_chw2hwc(tensor_t *chw, option_t *opt);

/// Tensor from hwc to chw
/// \param hwc
/// \param opt
/// \return
FUNCTION_IRAM tensor_t tensor_hwc2chw(tensor_t *hwc, option_t *opt);


#endif //CNET_TENSOR_H
