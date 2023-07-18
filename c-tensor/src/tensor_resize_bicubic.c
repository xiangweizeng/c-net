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

FUNCTION_IRAM static inline void interpolate_cubic(float fx, float* coeffs)
{
    const float A = -0.75f;

    float fx0 = fx + 1;
    float fx1 = fx;
    float fx2 = 1 - fx;
    // float fx3 = 2 - fx;

    coeffs[0] = A * fx0*fx0*fx0 - 5*A * fx0*fx0 + 8*A * fx0 - 4*A;
    coeffs[1] = (A+2) * fx1*fx1*fx1 - (A+3) * fx1*fx1 + 1;
    coeffs[2] = (A+2) * fx2*fx2*fx2 - (A+3) * fx2*fx2 + 1;
    coeffs[3] = 1.f - coeffs[0] - coeffs[1] - coeffs[2];
}

FUNCTION_IRAM static void cubic_coeffs(int w, int outw, int* xofs, float* alpha)
{
    double scale = (double)w / outw;

    for (int dx = 0; dx < outw; dx++)
    {
        float fx = (float)((dx + 0.5) * scale - 0.5);
        int sx = floor(fx);
        fx -= sx;

        interpolate_cubic(fx, alpha + dx*4);

        if (sx <= -1)
        {
            sx = 1;
            alpha[dx*4 +0] = 1.f - alpha[dx*4 +3];
            alpha[dx*4 +1] = alpha[dx*4 +3];
            alpha[dx*4 +2] = 0.f;
            alpha[dx*4 +3] = 0.f;
        }
        if (sx == 0)
        {
            sx = 1;
            alpha[dx*4 +0] = alpha[dx*4 +0] + alpha[dx*4 +1];
            alpha[dx*4 +1] = alpha[dx*4 +2];
            alpha[dx*4 +2] = alpha[dx*4 +3];
            alpha[dx*4 +3] = 0.f;
        }
        if (sx == w - 2)
        {
            sx = w - 3;
            alpha[dx*4 +3] = alpha[dx*4 +2] + alpha[dx*4 +3];
            alpha[dx*4 +2] = alpha[dx*4 +1];
            alpha[dx*4 +1] = alpha[dx*4 +0];
            alpha[dx*4 +0] = 0.f;
        }
        if (sx >= w - 1)
        {
            sx = w - 3;
            alpha[dx*4 +3] = 1.f - alpha[dx*4 +0];
            alpha[dx*4 +2] = alpha[dx*4 +0];
            alpha[dx*4 +1] = 0.f;
            alpha[dx*4 +0] = 0.f;
        }

        xofs[dx] = sx;
    }
}


static void resize_bicubic_image_int8(const tensor_t *src, tensor_t *dst, float *alpha, int *xofs, float *beta, int *yofs) {
    int w = dst->d0;
    int h = dst->d1;

    // loop body
    tensor_t rowsbuf0 = tensor_create_default();
    tensor_create_1d(&rowsbuf0, w, 2u, option_get_instance().workspace);
    tensor_t rowsbuf1 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 2u, option_get_instance().workspace);
    tensor_t rowsbuf2 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 2u, option_get_instance().workspace);
    tensor_t rowsbuf3 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 2u, option_get_instance().workspace);
    int8_t *rows0 = rowsbuf0.data;
    int8_t *rows1 = rowsbuf1.data;
    int8_t *rows2 = rowsbuf2.data;
    int8_t *rows3 = rowsbuf3.data;

    int prev_sy1 = -3;

    for (int dy = 0; dy < h; dy++) {
        int sy = yofs[dy];

        if (sy == prev_sy1) {
            // reuse all rows
        } else if (sy == prev_sy1 + 1) {
            // hresize one row
            int8_t *rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows2;
            rows2 = rows3;
            rows3 = rows0_old;
            const int8_t *S3 = (const int8_t *) src->data + src->d0 * (sy + 2);

            const float *alphap = alpha;
            int8_t *rows3p = rows3;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int8_t *S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows3p[dx] = S3p[-1] * a0 + S3p[0] * a1 + S3p[1] * a2 + S3p[2] * a3;

                alphap += 4;
            }
        } else if (sy == prev_sy1 + 2) {
            // hresize two rows
            int8_t *rows0_old = rows0;
            int8_t *rows1_old = rows1;
            rows0 = rows2;
            rows1 = rows3;
            rows2 = rows0_old;
            rows3 = rows1_old;
            const int8_t *S2 = (const int8_t *) src->data + src->d0 * (sy + 1);
            const int8_t *S3 = (const int8_t *) src->data + src->d0 * (sy + 2);

            const float *alphap = alpha;
            int8_t *rows2p = rows2;
            int8_t *rows3p = rows3;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int8_t *S2p = S2 + sx;
                const int8_t *S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows2p[dx] = S2p[-1] * a0 + S2p[0] * a1 + S2p[1] * a2 + S2p[2] * a3;
                rows3p[dx] = S3p[-1] * a0 + S3p[0] * a1 + S3p[1] * a2 + S3p[2] * a3;

                alphap += 4;
            }
        } else if (sy == prev_sy1 + 3) {
            // hresize three rows
            int8_t *rows0_old = rows0;
            int8_t *rows1_old = rows1;
            int8_t *rows2_old = rows2;
            rows0 = rows3;
            rows1 = rows0_old;
            rows2 = rows1_old;
            rows3 = rows2_old;
            const int8_t *S1 = (const int8_t *) src->data + src->d0 * (sy);
            const int8_t *S2 = (const int8_t *) src->data + src->d0 * (sy + 1);
            const int8_t *S3 = (const int8_t *) src->data + src->d0 * (sy + 2);

            const float *alphap = alpha;
            int8_t *rows1p = rows1;
            int8_t *rows2p = rows2;
            int8_t *rows3p = rows3;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int8_t *S1p = S1 + sx;
                const int8_t *S2p = S2 + sx;
                const int8_t *S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows1p[dx] = S1p[-1] * a0 + S1p[0] * a1 + S1p[1] * a2 + S1p[2] * a3;
                rows2p[dx] = S2p[-1] * a0 + S2p[0] * a1 + S2p[1] * a2 + S2p[2] * a3;
                rows3p[dx] = S3p[-1] * a0 + S3p[0] * a1 + S3p[1] * a2 + S3p[2] * a3;

                alphap += 4;
            }
        } else {
            // hresize four rows
            const int8_t *S0 = (const int8_t *) src->data + src->d0 * (sy - 1);
            const int8_t *S1 = (const int8_t *) src->data + src->d0 * (sy);
            const int8_t *S2 = (const int8_t *) src->data + src->d0 * (sy + 1);
            const int8_t *S3 = (const int8_t *) src->data + src->d0 * (sy + 2);

            const float *alphap = alpha;
            int8_t *rows0p = rows0;
            int8_t *rows1p = rows1;
            int8_t *rows2p = rows2;
            int8_t *rows3p = rows3;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int8_t *S0p = S0 + sx;
                const int8_t *S1p = S1 + sx;
                const int8_t *S2p = S2 + sx;
                const int8_t *S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows0p[dx] = S0p[-1] * a0 + S0p[0] * a1 + S0p[1] * a2 + S0p[2] * a3;
                rows1p[dx] = S1p[-1] * a0 + S1p[0] * a1 + S1p[1] * a2 + S1p[2] * a3;
                rows2p[dx] = S2p[-1] * a0 + S2p[0] * a1 + S2p[1] * a2 + S2p[2] * a3;
                rows3p[dx] = S3p[-1] * a0 + S3p[0] * a1 + S3p[1] * a2 + S3p[2] * a3;

                alphap += 4;
            }
        }

        prev_sy1 = sy;

        // vresize
        float b0 = beta[0];
        float b1 = beta[1];
        float b2 = beta[2];
        float b3 = beta[3];

        int8_t *rows0p = rows0;
        int8_t *rows1p = rows1;
        int8_t *rows2p = rows2;
        int8_t *rows3p = rows3;
        int8_t *Dp = (int8_t *) dst->data + dst->d0 * dy;
        for (int dx = 0; dx < w; dx++) {
//             D[x] = rows0[x]*b0 + rows1[x]*b1 + rows2[x]*b2 + rows3[x]*b3;
            *Dp++ = *rows0p++ * b0 + *rows1p++ * b1 + *rows2p++ * b2 + *rows3p++ * b3;
        }

        beta += 4;
    }

    tensor_release(&rowsbuf0);
    tensor_release(&rowsbuf1);
    tensor_release(&rowsbuf2);
    tensor_release(&rowsbuf3);
}

static void resize_bicubic_image_int16(
        const tensor_t *src, tensor_t *dst, float *alpha, int *xofs, float *beta, int *yofs) {
    int w = dst->d0;
    int h = dst->d1;

    // loop body
    tensor_t rowsbuf0 = tensor_create_default();
    tensor_create_1d(&rowsbuf0, w, 2u, option_get_instance().workspace);
    tensor_t rowsbuf1 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 2u, option_get_instance().workspace);
    tensor_t rowsbuf2 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 2u, option_get_instance().workspace);
    tensor_t rowsbuf3 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 2u, option_get_instance().workspace);
    int16_t *rows0 = rowsbuf0.data;
    int16_t *rows1 = rowsbuf1.data;
    int16_t *rows2 = rowsbuf2.data;
    int16_t *rows3 = rowsbuf3.data;

    int prev_sy1 = -3;

    for (int dy = 0; dy < h; dy++) {
        int sy = yofs[dy];

        if (sy == prev_sy1) {
            // reuse all rows
        } else if (sy == prev_sy1 + 1) {
            // hresize one row
            int16_t *rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows2;
            rows2 = rows3;
            rows3 = rows0_old;
            const int16_t *S3 = (const int16_t *) src->data + src->d0 * (sy + 2);

            const float *alphap = alpha;
            int16_t *rows3p = rows3;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int16_t *S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows3p[dx] = S3p[-1] * a0 + S3p[0] * a1 + S3p[1] * a2 + S3p[2] * a3;

                alphap += 4;
            }
        } else if (sy == prev_sy1 + 2) {
            // hresize two rows
            int16_t *rows0_old = rows0;
            int16_t *rows1_old = rows1;
            rows0 = rows2;
            rows1 = rows3;
            rows2 = rows0_old;
            rows3 = rows1_old;
            const int16_t *S2 = (const int16_t *) src->data + src->d0 * (sy + 1);
            const int16_t *S3 = (const int16_t *) src->data + src->d0 * (sy + 2);

            const float *alphap = alpha;
            int16_t *rows2p = rows2;
            int16_t *rows3p = rows3;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int16_t *S2p = S2 + sx;
                const int16_t *S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows2p[dx] = S2p[-1] * a0 + S2p[0] * a1 + S2p[1] * a2 + S2p[2] * a3;
                rows3p[dx] = S3p[-1] * a0 + S3p[0] * a1 + S3p[1] * a2 + S3p[2] * a3;

                alphap += 4;
            }
        } else if (sy == prev_sy1 + 3) {
            // hresize three rows
            int16_t *rows0_old = rows0;
            int16_t *rows1_old = rows1;
            int16_t *rows2_old = rows2;
            rows0 = rows3;
            rows1 = rows0_old;
            rows2 = rows1_old;
            rows3 = rows2_old;
            const int16_t *S1 = (const int16_t *) src->data + src->d0 * (sy);
            const int16_t *S2 = (const int16_t *) src->data + src->d0 * (sy + 1);
            const int16_t *S3 = (const int16_t *) src->data + src->d0 * (sy + 2);

            const float *alphap = alpha;
            int16_t *rows1p = rows1;
            int16_t *rows2p = rows2;
            int16_t *rows3p = rows3;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int16_t *S1p = S1 + sx;
                const int16_t *S2p = S2 + sx;
                const int16_t *S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows1p[dx] = S1p[-1] * a0 + S1p[0] * a1 + S1p[1] * a2 + S1p[2] * a3;
                rows2p[dx] = S2p[-1] * a0 + S2p[0] * a1 + S2p[1] * a2 + S2p[2] * a3;
                rows3p[dx] = S3p[-1] * a0 + S3p[0] * a1 + S3p[1] * a2 + S3p[2] * a3;

                alphap += 4;
            }
        } else {
            // hresize four rows
            const int16_t *S0 = (const int16_t *) src->data + src->d0 * (sy - 1);
            const int16_t *S1 = (const int16_t *) src->data + src->d0 * (sy);
            const int16_t *S2 = (const int16_t *) src->data + src->d0 * (sy + 1);
            const int16_t *S3 = (const int16_t *) src->data + src->d0 * (sy + 2);

            const float *alphap = alpha;
            int16_t *rows0p = rows0;
            int16_t *rows1p = rows1;
            int16_t *rows2p = rows2;
            int16_t *rows3p = rows3;
            for (int dx = 0; dx < w; dx++) {
                int sx = xofs[dx];
                const int16_t *S0p = S0 + sx;
                const int16_t *S1p = S1 + sx;
                const int16_t *S2p = S2 + sx;
                const int16_t *S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows0p[dx] = S0p[-1] * a0 + S0p[0] * a1 + S0p[1] * a2 + S0p[2] * a3;
                rows1p[dx] = S1p[-1] * a0 + S1p[0] * a1 + S1p[1] * a2 + S1p[2] * a3;
                rows2p[dx] = S2p[-1] * a0 + S2p[0] * a1 + S2p[1] * a2 + S2p[2] * a3;
                rows3p[dx] = S3p[-1] * a0 + S3p[0] * a1 + S3p[1] * a2 + S3p[2] * a3;

                alphap += 4;
            }
        }

        prev_sy1 = sy;

        // vresize
        float b0 = beta[0];
        float b1 = beta[1];
        float b2 = beta[2];
        float b3 = beta[3];

        int16_t *rows0p = rows0;
        int16_t *rows1p = rows1;
        int16_t *rows2p = rows2;
        int16_t *rows3p = rows3;
        int16_t *Dp = (int16_t *) dst->data + dst->d0 * dy;
        for (int dx = 0; dx < w; dx++) {
//             D[x] = rows0[x]*b0 + rows1[x]*b1 + rows2[x]*b2 + rows3[x]*b3;
            *Dp++ = *rows0p++ * b0 + *rows1p++ * b1 + *rows2p++ * b2 + *rows3p++ * b3;
        }

        beta += 4;
    }

    tensor_release(&rowsbuf0);
    tensor_release(&rowsbuf1);
    tensor_release(&rowsbuf2);
    tensor_release(&rowsbuf3);
}

FUNCTION_IRAM static void resize_bicubic_image_float(
        const tensor_t * src, tensor_t * dst, float* alpha, int* xofs, float* beta, int* yofs)
{
    int w = dst->d0;
    int h = dst->d1;

    // loop body
    tensor_t rowsbuf0 = tensor_create_default();
    tensor_create_1d(&rowsbuf0, w, 4u, option_get_instance().workspace);
    tensor_t rowsbuf1 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 4u, option_get_instance().workspace);
    tensor_t rowsbuf2 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 4u, option_get_instance().workspace);
    tensor_t rowsbuf3 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, 4u, option_get_instance().workspace);
    float* rows0 = rowsbuf0.data;
    float* rows1 = rowsbuf1.data;
    float* rows2 = rowsbuf2.data;
    float* rows3 = rowsbuf3.data;

    int prev_sy1 = -3;

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
            rows1 = rows2;
            rows2 = rows3;
            rows3 = rows0_old;
            const float* S3 =  (const float*)src->data + src->d0*(sy+2);

            const float* alphap = alpha;
            float* rows3p = rows3;
            for (int dx = 0; dx < w; dx++)
            {
                int sx = xofs[dx];
                const float* S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows3p[dx] = S3p[-1]*a0 + S3p[0]*a1 + S3p[1]*a2 + S3p[2]*a3;

                alphap += 4;
            }
        }
        else if (sy == prev_sy1 + 2)
        {
            // hresize two rows
            float* rows0_old = rows0;
            float* rows1_old = rows1;
            rows0 = rows2;
            rows1 = rows3;
            rows2 = rows0_old;
            rows3 = rows1_old;
            const float* S2 =  (const float*)src->data + src->d0*(sy+1);
            const float* S3 =  (const float*)src->data + src->d0*(sy+2);

            const float* alphap = alpha;
            float* rows2p = rows2;
            float* rows3p = rows3;
            for (int dx = 0; dx < w; dx++)
            {
                int sx = xofs[dx];
                const float* S2p = S2 + sx;
                const float* S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows2p[dx] = S2p[-1]*a0 + S2p[0]*a1 + S2p[1]*a2 + S2p[2]*a3;
                rows3p[dx] = S3p[-1]*a0 + S3p[0]*a1 + S3p[1]*a2 + S3p[2]*a3;

                alphap += 4;
            }
        }
        else if (sy == prev_sy1 + 3)
        {
            // hresize three rows
            float* rows0_old = rows0;
            float* rows1_old = rows1;
            float* rows2_old = rows2;
            rows0 = rows3;
            rows1 = rows0_old;
            rows2 = rows1_old;
            rows3 = rows2_old;
            const float* S1 =  (const float*)src->data + src->d0*(sy);
            const float* S2 =  (const float*)src->data + src->d0*(sy+1);
            const float* S3 =  (const float*)src->data + src->d0*(sy+2);

            const float* alphap = alpha;
            float* rows1p = rows1;
            float* rows2p = rows2;
            float* rows3p = rows3;
            for (int dx = 0; dx < w; dx++)
            {
                int sx = xofs[dx];
                const float* S1p = S1 + sx;
                const float* S2p = S2 + sx;
                const float* S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows1p[dx] = S1p[-1]*a0 + S1p[0]*a1 + S1p[1]*a2 + S1p[2]*a3;
                rows2p[dx] = S2p[-1]*a0 + S2p[0]*a1 + S2p[1]*a2 + S2p[2]*a3;
                rows3p[dx] = S3p[-1]*a0 + S3p[0]*a1 + S3p[1]*a2 + S3p[2]*a3;

                alphap += 4;
            }
        }
        else
        {
            // hresize four rows
            const float* S0 =  (const float*)src->data + src->d0*(sy-1);
            const float* S1 =  (const float*)src->data + src->d0*(sy);
            const float* S2 =  (const float*)src->data + src->d0*(sy+1);
            const float* S3 =  (const float*)src->data + src->d0*(sy+2);

            const float* alphap = alpha;
            float* rows0p = rows0;
            float* rows1p = rows1;
            float* rows2p = rows2;
            float* rows3p = rows3;
            for (int dx = 0; dx < w; dx++)
            {
                int sx = xofs[dx];
                const float* S0p = S0 + sx;
                const float* S1p = S1 + sx;
                const float* S2p = S2 + sx;
                const float* S3p = S3 + sx;

                float a0 = alphap[0];
                float a1 = alphap[1];
                float a2 = alphap[2];
                float a3 = alphap[3];
                rows0p[dx] = S0p[-1]*a0 + S0p[0]*a1 + S0p[1]*a2 + S0p[2]*a3;
                rows1p[dx] = S1p[-1]*a0 + S1p[0]*a1 + S1p[1]*a2 + S1p[2]*a3;
                rows2p[dx] = S2p[-1]*a0 + S2p[0]*a1 + S2p[1]*a2 + S2p[2]*a3;
                rows3p[dx] = S3p[-1]*a0 + S3p[0]*a1 + S3p[1]*a2 + S3p[2]*a3;

                alphap += 4;
            }
        }

        prev_sy1 = sy;

        // vresize
        float b0 = beta[0];
        float b1 = beta[1];
        float b2 = beta[2];
        float b3 = beta[3];

        float* rows0p = rows0;
        float* rows1p = rows1;
        float* rows2p = rows2;
        float* rows3p = rows3;
        float* Dp = (float*)dst->data + dst->d0 * dy;
        for (int dx = 0; dx < w; dx++)
        {
//             D[x] = rows0[x]*b0 + rows1[x]*b1 + rows2[x]*b2 + rows3[x]*b3;
            *Dp++ = *rows0p++ * b0 + *rows1p++ * b1 + *rows2p++ * b2 + *rows3p++ * b3;
        }

        beta += 4;
    }

    tensor_release(&rowsbuf0);
    tensor_release(&rowsbuf1);
    tensor_release(&rowsbuf2);
    tensor_release(&rowsbuf3);
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

FUNCTION_IRAM void interp_other_thread(interp_other_context_t *context, int q) {
    const tensor_t src = tensor_d2(context->bottom_tensor, q);
    tensor_t dst = tensor_d2(context->top_tensor, q);

    context->resize(&src, &dst, context->alpha, context->xofs, context->beta, context->yofs);
}

tensor_t tensor_resize_bicubic(
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
    tensor_create_3d(&dst,  ow, oh, c, elem_size, opt->tensor);
    if (tensor_empty(&dst)){
        return dst;
    }

    int* buf = (int*)fast_malloc((ow + oh + ow*4 + oh*4)*sizeof(int));

    int* xofs = buf;//new int[ow];
    int* yofs = buf + ow;//new int[oh];

    float* alpha = (float*)(buf + ow + oh);//new float[ow * 4];
    float* beta = (float*)(buf + ow + oh + ow*4);//new float[oh * 4];

    cubic_coeffs(w, ow, xofs, alpha);
    cubic_coeffs(h, oh, yofs, beta);

    if(elem_size == 1){
        interp_other_context_t context = {src, &dst, xofs, yofs, alpha, beta, resize_bicubic_image_int8};
        PARALLELIZE_1D(interp_other_thread, context, c);
    }else if(elem_size == 2){
        interp_other_context_t context = {src, &dst, xofs, yofs, alpha, beta, resize_bicubic_image_int16};
        PARALLELIZE_1D(interp_other_thread, context, c);
    }else if(elem_size == 4){
        interp_other_context_t context = {src, &dst, xofs, yofs, alpha, beta, resize_bicubic_image_float};
        PARALLELIZE_1D(interp_other_thread, context, c);
    }

    fast_free(buf);
    return dst;
}