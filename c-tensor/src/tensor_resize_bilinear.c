/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/


#include "math.h"
#include "tensor_operation.h"

#undef Min
#undef Max
#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))

FUNCTION_IRAM static void linear_coeffs(int w, int outw, int* xofs, float* alpha)
{
    double scale = (double)w / outw;

    for (int dx = 0; dx < outw; dx++)
    {
        float fx = (float)((dx + 0.5) * scale - 0.5);
        int sx = floor(fx);
        fx -= sx;

        if (sx < 0)
        {
            sx = 0;
            fx = 0.f;
        }
        if (sx >= w - 1)
        {
            sx = w - 2;
            fx = 1.f;
        }

        xofs[dx] = sx;

        alpha[dx*2    ] = 1.f - fx;
        alpha[dx*2 + 1] = fx;
    }
}



FUNCTION_IRAM static void resize_bilinear_image_int8(
        const tensor_t *src,
        tensor_t *dst,
        float *alpha,
        int *xofs,
        float *beta,
        int *yofs) {

    int w = dst->d0;
    int h = dst->d1;

    // loop body
    tensor_t rowsbuf0 = tensor_create_default();
    tensor_create_1d(&rowsbuf0, w, 2u, option_get_instance().workspace);
    tensor_t rowsbuf1 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 2u, option_get_instance().workspace);
    int8_t *rows0 = rowsbuf0.data;
    int8_t *rows1 = rowsbuf1.data;

    int prev_sy1 = -2;

    for (int dy = 0; dy < h; dy++) {
        int sy = yofs[dy];

        if (sy == prev_sy1) {
            // reuse all rows
        } else if (sy == prev_sy1 + 1) {
            // hresize one row
            int8_t *rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const int8_t *S1 = (const int8_t *) src->data + src->d0 * (sy + 1);

            const float *alphap = alpha;
            int8_t *rows1p = rows1;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int8_t *S1p = S1 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                rows1p[dx] = S1p[0] * a0 + S1p[1] * a1;

                alphap += 2;
            }
        } else {
            // hresize two rows
            const int8_t *S0 = (const int8_t *) src->data + src->d0 * (sy);
            const int8_t *S1 = (const int8_t *) src->data + src->d0 * (sy + 1);

            const float *alphap = alpha;
            int8_t *rows0p = rows0;
            int8_t *rows1p = rows1;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int8_t *S0p = S0 + sx;
                const int8_t *S1p = S1 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                rows0p[dx] = S0p[0] * a0 + S0p[1] * a1;
                rows1p[dx] = S1p[0] * a0 + S1p[1] * a1;

                alphap += 2;
            }
        }

        prev_sy1 = sy;

        // vresize
        float b0 = beta[0];
        float b1 = beta[1];

        int8_t *rows0p = rows0;
        int8_t *rows1p = rows1;
        int8_t *Dp = (int8_t *) dst->data + dst->d0 * dy;
        for (int dx = 0; dx < w; dx++) {
//             D[x] = rows0[x]*b0 + rows1[x]*b1;
            *Dp++ = *rows0p++ * b0 + *rows1p++ * b1;
        }

        beta += 2;
    }

    tensor_release(&rowsbuf0);
    tensor_release(&rowsbuf1);
}

FUNCTION_IRAM static void resize_bilinear_image_int16(
        const tensor_t *src,
        tensor_t *dst,
        float *alpha,
        int *xofs,
        float *beta,
        int *yofs) {

    int w = dst->d0;
    int h = dst->d1;

    // loop body
    tensor_t rowsbuf0 = tensor_create_default();
    tensor_create_1d(&rowsbuf0, w, 2u, option_get_instance().workspace);
    tensor_t rowsbuf1 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 2u, option_get_instance().workspace);
    int16_t *rows0 = rowsbuf0.data;
    int16_t *rows1 = rowsbuf1.data;

    int prev_sy1 = -2;

    for (int dy = 0; dy < h; dy++) {
        int sy = yofs[dy];

        if (sy == prev_sy1) {
            // reuse all rows
        } else if (sy == prev_sy1 + 1) {
            // hresize one row
            int16_t *rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const int16_t *S1 = (const int16_t *) src->data + src->d0 * (sy + 1);

            const float *alphap = alpha;
            int16_t *rows1p = rows1;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int16_t *S1p = S1 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                rows1p[dx] = S1p[0] * a0 + S1p[1] * a1;

                alphap += 2;
            }
        } else {
            // hresize two rows
            const int16_t *S0 = (const int16_t *) src->data + src->d0 * (sy);
            const int16_t *S1 = (const int16_t *) src->data + src->d0 * (sy + 1);

            const float *alphap = alpha;
            int16_t *rows0p = rows0;
            int16_t *rows1p = rows1;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int16_t *S0p = S0 + sx;
                const int16_t *S1p = S1 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                rows0p[dx] = S0p[0] * a0 + S0p[1] * a1;
                rows1p[dx] = S1p[0] * a0 + S1p[1] * a1;

                alphap += 2;
            }
        }

        prev_sy1 = sy;

        // vresize
        float b0 = beta[0];
        float b1 = beta[1];

        int16_t *rows0p = rows0;
        int16_t *rows1p = rows1;
        int16_t *Dp = (int16_t *) dst->data + dst->d0 * dy;
        for (int dx = 0; dx < w; dx++) {
//             D[x] = rows0[x]*b0 + rows1[x]*b1;
            *Dp++ = *rows0p++ * b0 + *rows1p++ * b1;
        }

        beta += 2;
    }

    tensor_release(&rowsbuf0);
    tensor_release(&rowsbuf1);
}

FUNCTION_IRAM static void resize_bilinear_image_float(const tensor_t * src, tensor_t * dst, float* alpha, int* xofs, float* beta, int* yofs)
{
    int w = dst->d0;
    int h = dst->d1;

    // loop body
    tensor_t rowsbuf0 = tensor_create_default();
    tensor_create_1d(&rowsbuf0, w, 4u, option_get_instance().workspace);
    tensor_t rowsbuf1 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 4u, option_get_instance().workspace);
    float* rows0 = rowsbuf0.data;
    float* rows1 = rowsbuf1.data;

    int prev_sy1 = -2;

    for (int dy = 0; dy < h; dy++ )
    {
        int sy = yofs[dy];

        if (sy == prev_sy1)
        {
            // reuse all rows
        }
        else if (sy == prev_sy1 + 1)
        {
            // hresize one row
            float* rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const float* S1 = (const float*)src->data + src->d0 *(sy + 1);

            const float* alphap = alpha;
            float* rows1p = rows1;
            for (int dx = 0; dx < w; dx++)
            {
                int sx = xofs[dx];
                const float* S1p = S1 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                rows1p[dx] = S1p[0]*a0 + S1p[1]*a1;

                alphap += 2;
            }
        }
        else
        {
            // hresize two rows
            const float* S0 = (const float*)src->data + src->d0 *(sy);
            const float* S1 = (const float*)src->data + src->d0 *(sy + 1);

            const float* alphap = alpha;
            float* rows0p = rows0;
            float* rows1p = rows1;
            for (int dx = 0; dx < w; dx++)
            {
                int sx = xofs[dx];
                const float* S0p = S0 + sx;
                const float* S1p = S1 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                rows0p[dx] = S0p[0]*a0 + S0p[1]*a1;
                rows1p[dx] = S1p[0]*a0 + S1p[1]*a1;

                alphap += 2;
            }
        }

        prev_sy1 = sy;

        // vresize
        float b0 = beta[0];
        float b1 = beta[1];

        float* rows0p = rows0;
        float* rows1p = rows1;
        float* Dp = (float*)dst->data + dst->d0 *dy;
        for (int dx = 0; dx < w; dx++)
        {
//             D[x] = rows0[x]*b0 + rows1[x]*b1;
            *Dp++ = *rows0p++ * b0 + *rows1p++ * b1;
        }

        beta += 2;
    }

    tensor_release(&rowsbuf0);
    tensor_release(&rowsbuf1);
}


typedef void (*resize_image)(const tensor_t* , tensor_t *, float*, int*, float*, int*);
typedef struct interp_other_context_t{
    tensor_t *bottom_tensor;
    tensor_t *top_tensor;
    int *xofs;
    int *yofs;
    float *alpha;
    float *beta;
    resize_image resize;
}interp_other_context_t;

FUNCTION_IRAM static void interp_other_thread(interp_other_context_t *context, int q) {
    const tensor_t src = tensor_d2(context->bottom_tensor, q);
    tensor_t dst = tensor_d2(context->top_tensor, q);
    context->resize(&src, &dst, context->alpha, context->xofs, context->beta, context->yofs);
}

tensor_t tensor_resize_bilinear(
        tensor_t *src,
        int ow, int oh,
        option_t* opt){

    int h = src->d1;
    int w = src->d0;
    int c = src->d2;
    size_t elem_size = src->elem_size;

    if (w == oh && w == ow)
    {
        tensor_add_ref(src);
        return *src;
    }

    tensor_t dst = tensor_create_default();
    tensor_create_3d(&dst,  ow, oh, c, elem_size, opt ? opt->tensor : NULL);
    if (tensor_empty(&dst)){
        return dst;
    }

    int* buf  = (int*)fast_malloc((ow + oh + ow*2 + oh*2) *sizeof(int));
    int* xofs = buf;//new int[ow];
    int* yofs = buf + ow;//new int[oh];

    float* alpha = (float*)(buf + ow + oh);//new float[ow * 2];
    float* beta = (float*)(buf + ow + oh + ow*2);//new float[oh * 2];

    linear_coeffs(w, ow, xofs, alpha);
    linear_coeffs(h, oh, yofs, beta);

    if(elem_size == 1){
        interp_other_context_t context = {src, &dst, xofs, yofs, alpha, beta, resize_bilinear_image_int8};
        PARALLELIZE_1D(interp_other_thread, context, c);
    }else if(elem_size == 2){
        interp_other_context_t context = {src, &dst, xofs, yofs, alpha, beta, resize_bilinear_image_int16};
        PARALLELIZE_1D(interp_other_thread, context, c);
    }else if(elem_size == 4){
        interp_other_context_t context = {src, &dst, xofs, yofs, alpha, beta, resize_bilinear_image_float};
        PARALLELIZE_1D(interp_other_thread, context, c);
    }

    fast_free(buf);
    return dst;
}