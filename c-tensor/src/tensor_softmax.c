/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/


#include <float.h>
#include <math.h>
#include <tensor.h>
#include <option.h>

#undef Min
#undef Max
#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))

int tensor_softmax(int axis, tensor_t *inplace_tensor, option_t *opt) {
    
    int dims = inplace_tensor->dims;
    size_t elemsize = inplace_tensor->elem_size;

    if (dims == 1) // axis == 0
    {
        int w = inplace_tensor->d0;

        float *ptr = (float *) inplace_tensor->data;

        float max = -FLT_MAX;
        for (int i = 0; i < w; i++) {
            max = Max(max, ptr[i]);
        }

        float sum = 0.f;
        for (int i = 0; i < w; i++) {
            ptr[i] = exp(ptr[i] - max);
            sum += ptr[i];
        }

        for (int i = 0; i < w; i++) {
            ptr[i] /= sum;
        }

        return CNET_STATUS_SUCCESS;
    }

    if (dims == 2 && axis == 0) {
        int w = inplace_tensor->d0;
        int h = inplace_tensor->d1;

        tensor_t max = tensor_create_default();
        tensor_create_1d(&max, w, elemsize, opt->workspace);
        if (tensor_empty(&max))
            return CNET_MEMORY_ALLOCATION_FAILED;

        tensor_fill_f(&max, -FLT_MAX);
        float *max_data = (float *) max.data;
        for (int i = 0; i < h; i++) {
            const float *ptr = (const float *) inplace_tensor->data + i * w;
            for (int j = 0; j < w; j++) {
                max_data[j] = Max(max_data[j], ptr[j]);
            }
        }

        tensor_t sum = tensor_create_default();
        tensor_create_1d(&sum, w, elemsize, opt->workspace);
        if (tensor_empty(&sum))
            return CNET_MEMORY_ALLOCATION_FAILED;

        tensor_fill_f(&sum, 0.f);
        float *sum_data = (float *) sum.data;
        for (int i = 0; i < h; i++) {
            float *ptr = (float *) inplace_tensor->data + i * w;
            for (int j = 0; j < w; j++) {
                ptr[j] = exp(ptr[j] - max_data[j]);
                sum_data[j] += ptr[j];
            }
        }

        for (int i = 0; i < h; i++) {
            float *ptr = (float *) inplace_tensor->data + i * w;
            for (int j = 0; j < w; j++) {
                ptr[j] /= sum_data[j];
            }
        }

        tensor_release(&max);
        tensor_release(&sum);
        return CNET_STATUS_SUCCESS;
    }

    if (dims == 2 && axis == 1) {
        int w = inplace_tensor->d0;
        int h = inplace_tensor->d1;

        for (int i = 0; i < h; i++) {
            float *ptr = (float *) inplace_tensor->data + i * w;
            float m = -FLT_MAX;
            for (int j = 0; j < w; j++) {
                m = Max(m, ptr[j]);
            }

            float s = 0.f;
            for (int j = 0; j < w; j++) {
                ptr[j] = exp(ptr[j] - m);
                s += ptr[j];
            }

            for (int j = 0; j < w; j++) {
                ptr[j] /= s;
            }
        }

        return CNET_STATUS_SUCCESS;
    }

    if (dims == 3 && axis == 0) {
        int w = inplace_tensor->d0;
        int h = inplace_tensor->d1;
        int channels = inplace_tensor->d2;
        int size = w * h;

        tensor_t max = tensor_create_default();
        tensor_create_2d(&max, w, h, elemsize, opt->workspace);
        if (tensor_empty(&max))
            return CNET_MEMORY_ALLOCATION_FAILED;

        tensor_fill_f(&max, -FLT_MAX);
        float *max_data = (float *) max.data;
        for (int q = 0; q < channels; q++) {
            tensor_t temp = tensor_d2(inplace_tensor, q);
            const float *ptr = (const float *) temp.data;

            for (int i = 0; i < size; i++) {
                max_data[i] = Max(max_data[i], ptr[i]);
            }
        }

        tensor_t sum = tensor_create_default();
        tensor_create_2d(&sum, w, h, elemsize, opt->workspace);
        if (tensor_empty(&sum))
            return CNET_MEMORY_ALLOCATION_FAILED;

        tensor_fill_f(&sum, 0.f);
        float *sum_data = (float *) sum.data;
        for (int q = 0; q < channels; q++) {
            tensor_t temp = tensor_d2(inplace_tensor, q);
            float *ptr = (float *) temp.data;

            for (int i = 0; i < size; i++) {
                ptr[i] = exp(ptr[i] - max_data[i]);
                sum_data[i] += ptr[i];
            }
        }

        for (int q = 0; q < channels; q++) {
            tensor_t temp = tensor_d2(inplace_tensor, q);
            float *ptr = (float *) temp.data;

            for (int i = 0; i < size; i++) {
                ptr[i] /= sum_data[i];
            }
        }

        tensor_release(&max);
        tensor_release(&sum);
        return CNET_STATUS_SUCCESS;
    }

    if (dims == 3 && axis == 1) {
        int w = inplace_tensor->d0;
        int h = inplace_tensor->d1;
        int channels = inplace_tensor->d2;

        tensor_t max = tensor_create_default();
        tensor_create_2d(&max, w, channels, elemsize, opt->workspace);
        if (tensor_empty(&max))
            return CNET_MEMORY_ALLOCATION_FAILED;

        tensor_fill_f(&max, -FLT_MAX);
        float *max_data = (float *) max.data;
        for (int q = 0; q < channels; q++) {
            tensor_t temp = tensor_d2(inplace_tensor, q);
            const float *ptr = (const float *) temp.data;

            float *maxptr = max_data + max.d0 * q;

            for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    maxptr[j] = Max(maxptr[j], ptr[j]);
                }

                ptr += w;
            }
        }

        tensor_t sum = tensor_create_default();
        tensor_create_2d(&sum, w, channels, elemsize, opt->workspace);
        if (tensor_empty(&sum))
            return CNET_MEMORY_ALLOCATION_FAILED;

        tensor_fill_f(&sum, 0.f);
        float *sum_data = (float *) sum.data;

        for (int q = 0; q < channels; q++) {
            tensor_t temp = tensor_d2(inplace_tensor, q);
            float *ptr = (float *) temp.data;

            float *maxptr = max_data + max.d0 * q;
            float *sumptr = sum_data + sum.d0 * q;

            for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    ptr[j] = exp(ptr[j] - maxptr[j]);
                    sumptr[j] += ptr[j];
                }

                ptr += w;
            }
        }

        for (int q = 0; q < channels; q++) {
            tensor_t temp = tensor_d2(inplace_tensor, q);
            float *ptr = (float *) temp.data;
            float *sumptr = sum_data + sum.d0 * q;

            for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    ptr[j] /= sumptr[j];
                }

                ptr += w;
            }
        }

        tensor_release(&max);
        tensor_release(&sum);
        return CNET_STATUS_SUCCESS;
    }

    if (dims == 3 && axis == 2) {
        int w = inplace_tensor->d0;
        int h = inplace_tensor->d1;
        int channels = inplace_tensor->d2;

        for (int q = 0; q < channels; q++) {
            tensor_t temp = tensor_d2(inplace_tensor, q);
            float *ptr = (float *) temp.data;

            for (int i = 0; i < h; i++) {
                float max = -FLT_MAX;
                for (int j = 0; j < w; j++) {
                    max = Max(max, ptr[j]);
                }

                float sum = 0.f;
                for (int j = 0; j < w; j++) {
                    ptr[j] = exp(ptr[j] - max);
                    sum += ptr[j];
                }

                for (int j = 0; j < w; j++) {
                    ptr[j] /= sum;
                }

                ptr += w;
            }
        }

        return CNET_STATUS_SUCCESS;
    }

    return CNET_STATUS_SUCCESS;
}
