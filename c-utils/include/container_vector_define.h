/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#if !defined(VECTOR_DEFINE_)
#define VECTOR_DEFINE_

#include <string.h>
#include "status.h"
#include "allocator.h"

#define VECTOR_EXPAND_SIZE 5

#define VECTOR_DECLARE(type, name) vector_##type name
#define VECTOR_CREATE(type, size) new_vector_##type##_impl(size)

/**
 * Define Vector
 */
#define VECTOR_DEFINE(type) \
typedef void element_##type##_operation(type *data, void *args);\
typedef struct vector##type##_ \
{\
    long length;\
    long current_size;\
    type *data;\
    void (*free_data)(struct vector##type##_ *);\
    int (*get_element)(struct vector##type##_ *, long index, type *dest);\
    int (*push_element)(struct vector##type##_ *, type *dest);\
    int (*set_element)(struct vector##type##_ *, long index, type *dest);\
    void(*for_each_element)(struct vector##type##_ *, element_##type##_operation, void *args);\
    malloc_func malloc;\
    free_func free;\
}vector_##type, *vector_##type##_ptr;\
vector_##type new_vector_##type##_impl(long length);

/**
 * Impl Vector
 */
#define VECTOR_IMPL(type) \
FUNCTION_IRAM void free_vector_##type##_data_impl(vector_##type##_ptr vec);\
FUNCTION_IRAM int get_vector_##type##_element_impl(vector_##type##_ptr vec, long index, type *dest);\
FUNCTION_IRAM int set_vector_##type##_element_impl(vector_##type##_ptr vec, long index, type *dest);\
FUNCTION_IRAM int push_vector_##type##_element_impl(vector_##type##_ptr vec, type *dest);\
FUNCTION_IRAM void for_each_vector_##type##_element_impl(vector_##type##_ptr vec, element_##type##_operation, void*);\
\
vector_##type new_vector_##type##_impl(long length){\
    vector_##type vector;\
    \
    vector.data = NULL;\
    vector.current_size = 0;\
    vector.length = 0;\
    vector.free_data = &free_vector_##type##_data_impl;\
    vector.get_element = &get_vector_##type##_element_impl;\
    vector.push_element = &push_vector_##type##_element_impl;\
    vector.set_element = &set_vector_##type##_element_impl;\
    vector.for_each_element = &for_each_vector_##type##_element_impl;\
    vector.malloc = allocator_get_malloc_fun(NULL, 0 == (sizeof(type) % 4));\
    vector.free = allocator_get_free_fun(NULL, 0 == (sizeof(type) % 4));\
    \
    if (length > 0 ){\
        type *data = (type*)vector.malloc(length * sizeof(type));\
        if (data){\
            vector.data = data;\
            vector.length = length;\
        }\
    }\
    \
    return vector;\
}\
\
void free_vector_##type##_data_impl(vector_##type##_ptr vec){\
    if (NULL != vec->data){\
        vec->free(vec->data);\
        vec->data = NULL;\
        vec->current_size = 0;\
        vec->length = 0;\
    }\
}\
\
int get_vector_##type##_element_impl(vector_##type##_ptr vec, long index, type *dest){\
    if(index < 0 || index >= vec->current_size){\
        return CNET_STATUS_SUCCESS;\
    }\
\
    *dest = vec->data[index];\
    return CNET_STATUS_SUCCESS;\
}\
\
int set_vector_##type##_element_impl(vector_##type##_ptr vec, long index, type *src){\
    if(index < 0){\
        return CNET_STATUS_FAILED;\
    }\
    \
    if(index >= vec->length){\
        long new_length = index + 1;\
        type *data = (type*)vec->malloc(new_length * sizeof(type));\
        if (data){\
            memcpy(data, vec->data, sizeof(type) * vec->current_size);\
            vec->free(vec->data);\
            vec->data = data;\
            vec->length = new_length;\
        }else{\
            return CNET_STATUS_FAILED;\
        }\
    }\
\
    if(vec->current_size <= index){\
        vec->current_size = index + 1;\
    }\
    vec->data[index] = *src;\
    return CNET_STATUS_SUCCESS;\
}\
\
int push_vector_##type##_element_impl(vector_##type##_ptr vec, type *src){\
    if(vec->current_size >= vec->length){\
        long new_length = vec->length + VECTOR_EXPAND_SIZE;\
        type *data = (type*)vec->malloc(new_length * sizeof(type));\
        if (data){\
            memcpy(data, vec->data, sizeof(type) * vec->current_size);\
            free(vec->data);\
            vec->data = data;\
            vec->length = new_length;\
        }else{\
            return CNET_STATUS_FAILED;\
        }\
    }\
\
    vec->data[vec->current_size] = *src;\
    vec->current_size ++;\
    return CNET_STATUS_SUCCESS;\
}\
\
void for_each_vector_##type##_element_impl(vector_##type##_ptr vec, element_##type##_operation operation, void *args){\
    int i;\
    for(i = 0; i < vec->current_size; i ++){\
        operation(vec->data + i, args);\
    }\
}


#endif // VECTOR_DEFINE_
