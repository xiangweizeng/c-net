
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"

FUNCTION_IRAM static int slice_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt)
{
    slice_t *slice = (slice_t *) operation->base;

    tensor_t *bottom_tensor = &bottom_tensors->data[0].data;
    int dims = bottom_tensor->dims;
    size_t elem_size = bottom_tensor->elem_size;
    const int* slices_ptr = (int*)slice->slices.data.data;

    /// axis == 0
    if (dims == 1)
    {
        int w = bottom_tensor->d0;

        int q = 0;
        for (size_t i=0; i<top_tensors->length; i++)
        {
            int slices = slices_ptr[i];
            if (slices == -233)
            {
                slices = (w - q) / (top_tensors->length - i);
            }

            tensor_t *top_tensor = &top_tensors->data[i].data;

            const int16_t* ptr = (const int16_t*)bottom_tensor + q;
            int16_t* out_ptr = (int16_t*)top_tensor->data;
            memcpy(out_ptr, ptr, slices * elem_size);

            q += slices;
        }

        return CNET_STATUS_SUCCESS;
    }

    if (dims == 2 && slice->config.axis == 0)
    {
        int w = bottom_tensor->d0;
        int h = bottom_tensor->d1;

        int q = 0;
        for (size_t i=0; i<top_tensors->length; i++)
        {
            int slices = slices_ptr[i];
            if (slices == -233)
            {
                slices = (h - q) / (top_tensors->length - i);
            }

            int size = w * slices;
            tensor_t *top_tensor = &top_tensors->data[i].data;
            const int16_t* ptr = (int16_t*)bottom_tensor->data + bottom_tensor->d0*q;
            int16_t* out_ptr = (int16_t*)top_tensor->data;
            memcpy(out_ptr, ptr, size * elem_size);

            q += slices;
        }

        return CNET_STATUS_SUCCESS;
    }

    if (dims == 2 && slice->config.axis == 1)
    {
        int w = bottom_tensor->d0;
        int h = bottom_tensor->d1;

        int q = 0;
        for (size_t i=0; i<top_tensors->length; i++)
        {
            int slices = slices_ptr[i];
            if (slices == -233)
            {
                slices = (w - q) / (top_tensors->length - i);
            }

            tensor_t *top_tensor = &top_tensors->data[i].data;
            for (int j=0; j<h; j++)
            {
                int16_t* out_ptr = (int16_t*)top_tensor->data + top_tensor->d0 * j;
                const int16_t* ptr = (int16_t*)bottom_tensor->data + bottom_tensor->d0 *j + q;
                memcpy(out_ptr, ptr, slices * elem_size);
            }

            q += slices;
        }

        return CNET_STATUS_SUCCESS;
    }

    if (dims == 3 && slice->config.axis == 0)
    {
        int channels = bottom_tensor->d2;

        int q = 0;
        for (size_t i=0; i<top_tensors->length; i++)
        {
            int slices = slices_ptr[i];
            if (slices == -233)
            {
                slices = (channels - q) / (top_tensors->length - i);
            }

            int size = bottom_tensor->step * slices;
            tensor_t *top_tensor = &top_tensors->data[i].data;

            const int16_t* ptr = (int16_t*)tensor_d2(bottom_tensor, q).data;
            int16_t* out_ptr = (int16_t*)top_tensor->data;
            memcpy(out_ptr, ptr, size * elem_size);

            q += slices;
        }

        return CNET_STATUS_SUCCESS;
    }

    if (dims == 3 && slice->config.axis == 1)
    {
        int w = bottom_tensor->d0;
        int h = bottom_tensor->d1;
        int channels = bottom_tensor->d2;

        int q = 0;
        for (size_t i=0; i<top_tensors->length; i++)
        {
            int slices = slices_ptr[i];
            if (slices == -233)
            {
                slices = (h - q) / (top_tensors->length - i);
            }

            tensor_t *top_tensor = &top_tensors->data[i].data;
            for (int p=0; p<channels; p++)
            {
                int size = w * slices;
                int16_t* out_ptr = (int16_t*)tensor_d2(top_tensor, p).data;
                tensor_t m = tensor_d2(bottom_tensor, p);
                const int16_t* ptr = (int16_t*)m.data + m.d0*q;
                memcpy(out_ptr, ptr, size * elem_size);
            }

            q += slices;
        }

        return CNET_STATUS_SUCCESS;
    }

    if (dims == 3 && slice->config.axis == 2)
    {
        int w = bottom_tensor->d0;
        int h = bottom_tensor->d1;
        int channels = bottom_tensor->d2;

        int q = 0;
        for (size_t i=0; i<top_tensors->length; i++)
        {
            int slices = slices_ptr[i];
            if (slices == -233)
            {
                slices = (w - q) / (top_tensors->length - i);
            }

            tensor_t *top_tensor = &top_tensors->data[i].data;
            for (int p=0; p<channels; p++)
            {
                int16_t* out_ptr = (int16_t*)tensor_d2(top_tensor, p).data;
                tensor_t m = tensor_d2(bottom_tensor, p);

                for (int j=0; j<h; j++)
                {
                    const int16_t* ptr = (int16_t*)m.data + m.d0*j + q;
                    memcpy(out_ptr, ptr, slices * elem_size);

                    out_ptr += slices;
                }
            }

            q += slices;
        }

        return CNET_STATUS_SUCCESS;
    }

    return CNET_STATUS_SUCCESS;

}

IMPL_OPERATION_CREATOR(slice) {
    operation_t *slice = (operation_t *)fast_malloc(sizeof(operation_t));
    if (NULL == slice) {
        printf("malloc for slice failed\n");
        return NULL;
    }

    operation_basic_info_setup(slice);
    slice->forward = slice_forward;
    return (operation_ptr) slice;
}
