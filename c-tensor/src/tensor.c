/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include <allocator.h>
#include "tensor.h"
#include "container_vector.h"
#include <stdio.h>

/**
 * vector impl for tensor_t
 * @param vec
 */
VECTOR_IMPL(tensor_t);

/**
 * tensor extra bates
 */
#define NHWC_EXTRA_BYTES 16

tensor_t tensor_create_default()
{
    tensor_t tensor;
    tensor.data = NULL;
    tensor.d3 = 0;
    tensor.d2 = 0;
    tensor.d1 = 0;
    tensor.dims = 0;
    tensor.d0 = 0;
    tensor.step = 0;
    tensor.refcount = NULL;
    tensor.elem_size = 0;
    tensor.layout = TENSOR_LAYOUT_NCHW;
    tensor.allocator = NULL;
    tensor.data_type = TENSOR_DATA_FLOAT;
    return  tensor;
}

vector_tensor_t vector_tensor_create_default(int tensor_count)
{
    VECTOR_DECLARE(tensor_t, tensors) = VECTOR_CREATE(tensor_t, tensor_count);
    tensors.current_size = tensor_count;
    tensor_t tensor = tensor_create_default();
    for(int i= 0; i < tensor_count; i++){
        tensors.data[i] = tensor;
    }

    return tensors;
}

void vector_tensor_free(vector_tensor_t *tensors)
{
    for(long i = 0; i < tensors->current_size; i++){
        tensor_release(tensors->data + i);
    }

    tensors->free_data(tensors);
}

#define PRINT_DATA(T,P,F)\
void print_data_##T(T*data, size_t w, size_t h, size_t c)\
{\
    for(int ic = 0; ic < c; ic ++){\
         printf("channel %d\n", ic);\
        for(int ih = 0 ; ih < h; ih ++){\
            T *pd = data + ic *w*h + ih*w;\
            for(int iw = 0; iw < w ; iw ++){\
                printf(F, (P)pd[iw]);\
            }\
            printf("\n");\
        }\
        printf("\n");\
    }\
}

PRINT_DATA(int8_t, int, "%d, ")
PRINT_DATA(uint8_t , int, "%d, ")
PRINT_DATA(float , float, "%f, ")
#if ESP32
PRINT_DATA(int64_t , int64_t, "%lld, ")
#else
PRINT_DATA(int64_t , int64_t, "%ld, ")
#endif

void tensor_print_u8(tensor_t *tensor){
    if(1 == tensor->elem_size){
        printf("\n");
        for(int c= 0; c< tensor->d2; c ++){
            printf("channel %d\n", c);
            tensor_t m = tensor_d2(tensor, c);
            uint8_t *data = (uint8_t*) m.data;
            for(int h = 0; h < m.d1; h++){
                for(int w = 0; w < m.d0; w ++){
                    printf("%d,", data[h*m.d0 + w]);
                    if((w+1) % 64 == 0) printf("\n");
                }

                printf("\n");
            }
        }
        printf("\n");
    }
}

void tensor_print_int32(tensor_t *tensor){
    if(4 == tensor->elem_size){
        printf("\n");
        for(int c= 0; c< tensor->d2; c ++){
            printf("channel %d\n", c);
            tensor_t m = tensor_d2(tensor, c);
            int32_t *data = (int32_t*) m.data;
            for(int h = 0; h < m.d1; h++){
                for(int w = 0; w < m.d0; w ++){
                    printf("%ld,", data[h*m.d0 + w]);
                }
                printf("\n");
            }
        }
        printf("\n");
    }
}

void tensor_print(tensor_t *tensor){

    if(tensor->layout == TENSOR_LAYOUT_NCHW){
        if(TENSOR_DATA_FLOAT == tensor->data_type){
            printf("\n");
            for(int c= 0; c< tensor->d2; c ++){
                printf("channel %d\n", c);
                float *data = (float*) tensor->data + c * tensor->d0 * tensor->d1;
                for(int h = 0; h < tensor->d1; h++){
                    for(int w = 0; w < tensor->d0; w ++){
                        printf("%.2f,", data[h*tensor->d0 + w]);
                    }

                    printf("\n");
                }
            }
            printf("\n");
        }else if(TENSOR_DATA_INT16 == tensor->data_type){
            printf("\n");
            for(int c= 0; c< tensor->d2; c ++){
                printf("channel %d\n", c);
                int16_t *data =  ((int16_t*)tensor->data) + c * tensor->d0 * tensor->d1;
                for(int h = 0; h < tensor->d1; h++){
                    for(int w = 0; w < tensor->d0; w ++){
                        printf("%d,", data[h*tensor->d0 + w]);
                    }

                    printf("\n");
                }
            }
            printf("\n");
        }else if(TENSOR_DATA_UINT8 == tensor->data_type){
            printf("\n");
            for(int c= 0; c< tensor->d2; c ++){
                printf("channel %d\n", c);
                uint8_t *data =  tensor->data + c * tensor->d0 * tensor->d1;
                for(int h = 0; h < tensor->d1; h++){
                    for(int w = 0; w < tensor->d0; w ++){
                        printf("%d,", data[h*tensor->d0 + w]);
                    }

                    printf("\n");
                }
            }
            printf("\n");
        }
    } else if(tensor->layout == TENSOR_LAYOUT_NHWC) {
        if (TENSOR_DATA_FLOAT == tensor->data_type) {
            printf("\n");
            float *data = (float *) tensor->data;
            for (int c = 0; c < tensor->d2; c++) {
                printf("channel %d\n", c);
                for (int h = 0; h < tensor->d1; h++) {
                    for (int w = 0; w < tensor->d0; w++) {
                        printf("%f,", data[(h * tensor->d0 + w) * tensor->d2 + c]);
                    }

                    printf("\n");
                }
            }
            printf("\n");

        } else if (TENSOR_DATA_INT8 == tensor->data_type) {
            printf("\n");
            char *data = tensor->data;
            for (int c = 0; c < tensor->d2; c++) {
                printf("channel %d\n", c);
                for (int h = 0; h < tensor->d1; h++) {
                    for (int w = 0; w < tensor->d0; w++) {
                        printf("%d,", data[(h * tensor->d0 + w) * tensor->d2 + c]);
                    }

                    printf("\n");
                }
            }
            printf("\n");
        } else if (TENSOR_DATA_UINT8 == tensor->data_type) {
            printf("\n");
            uint8_t *data = tensor->data;
            for (int c = 0; c < tensor->d2; c++) {
                printf("channel %d\n", c);
                for (int h = 0; h < tensor->d1; h++) {
                    for (int w = 0; w < tensor->d0; w++) {
                        printf("%d,", data[(h * tensor->d0 + w) * tensor->d2 + c]);
                    }

                    printf("\n");
                }
            }
            printf("\n");
        }
    }
}

void tensor_fill_f(tensor_t *tensor, float v) {
    float *data = (float *) tensor->data;
    size_t size = tensor_total(tensor);
    for (; size > 0; size--) {
        *data++ = v;
    }
}

void tensor_fill_i32(tensor_t *tensor, int v) {
    int *data = (int *) tensor->data;
    size_t size = tensor_total(tensor);
    for (; size > 0; size--) {
        *data++ = v;
    }
}

void tensor_fill_i8(tensor_t *tensor, char v) {
    char *data = (char *) tensor->data;
    size_t size = tensor_total(tensor);
    for (; size > 0; size--) {
        *data++ = v;
    }
}

void tensor_fill_u8(tensor_t *tensor, unsigned char v) {
    unsigned char *data = (unsigned char *) tensor->data;
    size_t size = tensor_total(tensor);
    for (; size > 0; size--) {
        *data++ = v;
    }
}

void tensor_set_data_type(tensor_t *tensor, tensor_data_type_t data_type)
{
    tensor->data_type = data_type;
}

FUNCTION_IRAM tensor_shape_t tensor_get_shape(tensor_t *tensor)
{
    tensor_shape_t shape;
    shape.dims = tensor->dims;

    shape.shape[0] = tensor->d3;
    shape.shape[1] = tensor->d2;
    shape.shape[2] = tensor->d1;
    shape.shape[3] = tensor->d0;
    return  shape;
}

FUNCTION_IRAM tensor_t tensor_clone(tensor_t *tensor) {
    if (tensor_empty(tensor)) {
        return tensor_create_default();
    }

    tensor_t temp = tensor_create_default();

    if (tensor->dims == 1)
        tensor_create_1d(&temp, tensor->d0, tensor->elem_size, tensor->allocator);
    else if (tensor->dims == 2)
        tensor_create_2d(&temp, tensor->d0, tensor->d1, tensor->elem_size, tensor->allocator);
    else if (tensor->dims == 3)
        tensor_create_3d(&temp, tensor->d0, tensor->d1, tensor->d2, tensor->elem_size, tensor->allocator);

    if (tensor_total(tensor) > 0) {
        memcpy(temp.data, tensor->data, tensor_total(tensor) * tensor->elem_size);
        temp.layout = tensor->layout;
        temp.step = tensor->step;
        temp.data_type = tensor->data_type;
    }

    return temp;
}

tensor_t tensor_reshape_1d(tensor_t *tensor, int d0) {
    if (tensor->d0 * tensor->d1 * tensor->d2 != d0 && tensor->layout == TENSOR_LAYOUT_NCHW) {
        return tensor_create_default();
    }

    tensor_t m = *tensor;
    tensor_add_ref(tensor);

    m.dims = 1;
    m.d0 = d0;
    m.d1 = 1;
    m.d2 = 1;
    m.d3 = 1;
    m.step = d0;

    return m;
}

tensor_t tensor_reshape_2d(tensor_t *tensor, int d0, int d1) {
    if (tensor->d0 * tensor->d1 * tensor->d2 != d0 * d1 && tensor->layout == TENSOR_LAYOUT_NCHW) {
        return tensor_create_default();
    }

    tensor_t m = *tensor;
    tensor_add_ref(tensor);

    m.dims = 2;
    m.d0 = d0;
    m.d1 = d1;
    m.d2 = 1;
    m.d3 = 1;

    m.step = d0 * d1;

    return m;
}

tensor_t tensor_reshape_3d(tensor_t *tensor, int w, int h, int c) {
    if (tensor->d0 * tensor->d1 * tensor->d2 != w * h * c) {
        return tensor_create_default();
    }

    tensor_t m = *tensor;
    tensor_add_ref(tensor);

    m.dims = 3;
    m.d0 = w;
    m.d1 = h;
    m.d2 = c;
    m.d3 = 1;

    m.step = w * h;

    return m;
}

void tensor_create_1d(tensor_t *tensor, int w, size_t elem_size, allocator_t *allocator) {
    if (tensor->dims == 1 && tensor->d0 == w && tensor->elem_size == elem_size &&
        tensor->layout == TENSOR_LAYOUT_NCHW && allocator == tensor->allocator){
        return;
    }

    tensor_release(tensor);
    tensor->allocator = allocator;
    tensor->elem_size = elem_size;

    tensor->dims = 1;
    tensor->d0 = w;
    tensor->d1 = 1;
    tensor->d2 = 1;
    tensor->d3 = 1;
    tensor->layout = TENSOR_LAYOUT_NCHW;

    tensor->step = w;

    if (tensor_total(tensor) > 0) {

        malloc_func malloc_fun = allocator_get_malloc_fun(tensor->allocator, tensor->elem_size == 4);
        size_t totalsize = tensor_total(tensor) * elem_size  + NHWC_EXTRA_BYTES;
        totalsize = align_size(totalsize, 4);
        tensor->data = malloc_fun(totalsize + (int) sizeof(*tensor->refcount));
        if(NULL != tensor->data){
            tensor->refcount = (int *) (((unsigned char *) tensor->data) + totalsize);
            *tensor->refcount = 1;
        }else{
            printf("alloc memory failed, %lu", (unsigned long int)totalsize);
        }
    }
}

// allocate image
void tensor_create_2d(tensor_t *tensor, int w, int h, size_t elem_size, allocator_t *allocator) {

    if (tensor->dims == 2 && tensor->d0 == w && tensor->d1 == h && tensor->elem_size == elem_size
        && tensor->layout == TENSOR_LAYOUT_NCHW && allocator == tensor->allocator){
        return;
    }

    tensor_release(tensor);
    tensor->allocator = allocator;
    tensor->elem_size = elem_size;

    tensor->dims = 2;
    tensor->d0 = w;
    tensor->d1 = h;
    tensor->d2 = 1;
    tensor->d3 = 1;
    tensor->layout = TENSOR_LAYOUT_NCHW;

    tensor->step = w * h;

    if (tensor_total(tensor) > 0) {

        malloc_func malloc_fun = allocator_get_malloc_fun(tensor->allocator, tensor->elem_size == 4);
        size_t totalsize = tensor_total(tensor) * elem_size  + NHWC_EXTRA_BYTES;
        totalsize = align_size(totalsize, 4);
        tensor->data = malloc_fun(totalsize + (int) sizeof(*tensor->refcount));
        if(NULL != tensor->data){
            tensor->refcount = (int *) (((unsigned char *) tensor->data) + totalsize);
            *tensor->refcount = 1;
        }else{
            printf("alloc memory failed, %lu", (unsigned long int)totalsize);
        }
    }
}

// allocate dim
void tensor_create_3d(tensor_t *tensor, int w, int h, int c, size_t elem_size, allocator_t *allocator) {
    if (tensor->dims == 3 && tensor->d0 == w && tensor->d1 == h && tensor->d2 == c
        && tensor->layout == TENSOR_LAYOUT_NCHW && tensor->elem_size == elem_size && tensor->allocator == allocator){
           return;
    }

    tensor_release(tensor);
    tensor->elem_size = elem_size;
    tensor->allocator = allocator;

    tensor->dims = 3;
    tensor->d0 = w;
    tensor->d1 = h;
    tensor->d2 = c;
    tensor->d3 = 1;
    tensor->layout = TENSOR_LAYOUT_NCHW;

    tensor->step = align_size(w * h * elem_size, elem_size) / elem_size;

    if (tensor_total(tensor) > 0) {
        size_t totalsize = tensor_total(tensor) * elem_size + NHWC_EXTRA_BYTES;
        totalsize = align_size(totalsize, 4);
        malloc_func malloc_fun = allocator_get_malloc_fun(tensor->allocator, tensor->elem_size == 4);
        tensor->data = malloc_fun(totalsize + (int) sizeof(*tensor->refcount));
        if(NULL != tensor->data){
            tensor->refcount = (int *) (((unsigned char *) tensor->data) + totalsize);
            *tensor->refcount = 1;
        }else{
            printf("alloc memory failed, %lu", (unsigned long int)totalsize);
        }
    }
}

void tensor_create_1d_data(tensor_t *tensor, int w, void *data, size_t elem_size) {
    tensor_release(tensor);
    tensor->dims = 1;
    tensor->d1 = 1;
    tensor->d2 = 1;
    tensor->d0 = w;
    tensor->data = data;
    tensor->allocator = NULL;
    tensor->elem_size = elem_size;
    tensor->refcount = 0;
    tensor->step = w;
    tensor->d3 = 1;
    tensor->layout = TENSOR_LAYOUT_NCHW;
}

// allocate image
void tensor_create_2d_data(tensor_t *tensor, int w, int h, void *data, size_t elem_size) {
    tensor_release(tensor);
    tensor->dims = 2;
    tensor->d1 = h;
    tensor->d2 = 1;
    tensor->d0 = w;
    tensor->data = data;
    tensor->allocator = NULL;
    tensor->elem_size = elem_size;
    tensor->refcount = 0;
    tensor->step = w * h;
    tensor->d3 = 1;
    tensor->layout = TENSOR_LAYOUT_NCHW;
}

// allocate dim
void tensor_create_3d_data(tensor_t *tensor, int w, int h, int c, void *data, size_t elem_size) {
    tensor_release(tensor);
    tensor->dims = 3;
    tensor->d1 = h;
    tensor->d2 = c;
    tensor->d0 = w;
    tensor->data = data;
    tensor->allocator = NULL;
    tensor->elem_size = elem_size;
    tensor->refcount = 0;
    tensor->step = w * h;
    tensor->d3 = 1;
    tensor->layout = TENSOR_LAYOUT_NCHW;
}

void tensor_create_like(tensor_t *tensor, const tensor_t *like, size_t element_size, allocator_t *allocator) {

    tensor_create_3d(tensor, like->d0, like->d1, like->d2, element_size, allocator);
    tensor->layout = like->layout;
    tensor->step = like->step;
    tensor->dims = like->dims;
}

// refcount++
inline void tensor_add_ref(tensor_t *tensor) {
    if (tensor->refcount)
        CNET_XADD(tensor->refcount, 1);
}

// refcount--
void tensor_release(tensor_t *tensor) {
    if (tensor->refcount && CNET_XADD(tensor->refcount, -1) == 1) {
        free_func free_fun = allocator_get_free_fun(tensor->allocator, tensor->elem_size == 4);
        free_fun(tensor->data);
    }

    tensor->data = 0;
    tensor->allocator = NULL;
    tensor->elem_size = 0;

    tensor->dims = 0;
    tensor->d0 = 0;
    tensor->d1 = 0;
    tensor->d2 = 0;
    tensor->d3 = 0;
    tensor->layout = TENSOR_LAYOUT_NCHW;

    tensor->step = 0;

    tensor->refcount = 0;
}

inline int tensor_empty(tensor_t *tensor) {
    return tensor->data == 0 || tensor_total(tensor) == 0;
}

inline size_t tensor_total(tensor_t *tensor) {
    return tensor->d0 * tensor->d1 * tensor->d2;
}

// data reference
tensor_t tensor_d2(tensor_t *tensor, int d2) {
    tensor_t temp = tensor_create_default();
    tensor_create_2d_data(&temp, tensor->d0, tensor->d1,
                          (uint8_t *) tensor->data + tensor->step * d2 * tensor->elem_size, tensor->elem_size);
    temp.layout = tensor->layout;
    temp.step = tensor->step;
    temp.data_type = tensor->data_type;
    return temp;
}

// range reference
tensor_t tensor_d2_range(tensor_t *tensor, int d2, int number) {
    tensor_t temp = tensor_create_default();
    tensor_create_3d_data(&temp, tensor->d0, tensor->d1, number,
                          (uint8_t *) tensor->data + tensor->step * d2 * tensor->elem_size, tensor->elem_size);
    temp.layout = tensor->layout;
    temp.step = tensor->step;
    temp.data_type = tensor->data_type;
    return temp;
}

tensor_t tensor_range(tensor_t *tensor, int x, int n) {
    tensor_t temp = tensor_create_default();
    tensor_create_1d_data(&temp, n, (uint8_t *) tensor->data + x * tensor->elem_size, tensor->elem_size);
    temp.layout = tensor->layout;
    temp.step = tensor->step;
    temp.data_type = tensor->data_type;
    return temp;
}

// convert half precision floating point to float
static float half2float(unsigned short value) {
    // 1 : 5 : 10
    unsigned short sign = (value & 0x8000) >> 15;
    unsigned short exponent = (value & 0x7c00) >> 10;
    unsigned short significand = value & 0x03FF;

//     fprintf(stderr, "%d %d %d\n", sign, exponent, significand);

    // 1 : 8 : 23
    union {
        unsigned int u;
        float f;
    } tmp;
    if (exponent == 0) {
        if (significand == 0) {
            // zero
            tmp.u = (sign << 31);
        } else {
            // denormal
            exponent = 0;
            // find non-zero bit
            while ((significand & 0x200) == 0) {
                significand <<= 1;
                exponent++;
            }
            significand <<= 1;
            significand &= 0x3FF;
            tmp.u = (sign << 31) | ((-exponent + (-15 + 127)) << 23) | (significand << 13);
        }
    } else if (exponent == 0x1F) {
        // infinity or NaN
        tmp.u = (sign << 31) | (0xFF << 23) | (significand << 13);
    } else {
        // normalized
        tmp.u = (sign << 31) | ((exponent + (-15 + 127)) << 23) | (significand << 13);
    }

    return tmp.f;
}

tensor_t tensor_from_float16(const unsigned short *data, int size, allocator_t *allocator) {
    tensor_t m = tensor_create_default();
    tensor_create_1d(&m, size, 4, allocator);
    if (tensor_empty(&m))
        return m;

    float *ptr = (float *) m.data;

    int remain = size;
    for (; remain > 0; remain--) {
        *ptr = half2float(*data);

        data++;
        ptr++;
    }

    return m;
}