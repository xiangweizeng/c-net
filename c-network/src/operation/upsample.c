
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "operation.h"
#include "operation_config.h"
#include "network.h"


#undef Min
#undef Max
#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))

FUNCTION_IRAM static void linear_coeffs(int w, int out_w, int *xofs, float *alpha) {
    double scale = (double) w / out_w;

    for (int dx = 0; dx < out_w; dx++) {
        float fx = (float) ((dx + 0.5) * scale - 0.5);
        int sx = floor(fx);
        fx -= sx;

        if (sx < 0) {
            sx = 0;
            fx = 0.f;
        }
        if (sx >= w - 1) {
            sx = w - 2;
            fx = 1.f;
        }

        xofs[dx] = sx;

        alpha[dx * 2] = 1.f - fx;
        alpha[dx * 2 + 1] = fx;
    }
}

FUNCTION_IRAM static void resize_bilinear_image(
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

FUNCTION_IRAM static inline void interpolate_cubic(float fx, float *coeffs) {
    const float A = -0.75f;

    float fx0 = fx + 1;
    float fx1 = fx;
    float fx2 = 1 - fx;
    // float fx3 = 2 - fx;

    coeffs[0] = A * fx0 * fx0 * fx0 - 5 * A * fx0 * fx0 + 8 * A * fx0 - 4 * A;
    coeffs[1] = (A + 2) * fx1 * fx1 * fx1 - (A + 3) * fx1 * fx1 + 1;
    coeffs[2] = (A + 2) * fx2 * fx2 * fx2 - (A + 3) * fx2 * fx2 + 1;
    coeffs[3] = 1.f - coeffs[0] - coeffs[1] - coeffs[2];
}

FUNCTION_IRAM static void cubic_coeffs(int w, int outw, int *xofs, float *alpha) {
    double scale = (double) w / outw;

    for (int dx = 0; dx < outw; dx++) {
        float fx = (float) ((dx + 0.5) * scale - 0.5);
        int sx = floor(fx);
        fx -= sx;

        interpolate_cubic(fx, alpha + dx * 4);

        if (sx <= -1) {
            sx = 1;
            alpha[dx * 4 + 0] = 1.f - alpha[dx * 4 + 3];
            alpha[dx * 4 + 1] = alpha[dx * 4 + 3];
            alpha[dx * 4 + 2] = 0.f;
            alpha[dx * 4 + 3] = 0.f;
        }
        if (sx == 0) {
            sx = 1;
            alpha[dx * 4 + 0] = alpha[dx * 4 + 0] + alpha[dx * 4 + 1];
            alpha[dx * 4 + 1] = alpha[dx * 4 + 2];
            alpha[dx * 4 + 2] = alpha[dx * 4 + 3];
            alpha[dx * 4 + 3] = 0.f;
        }
        if (sx == w - 2) {
            sx = w - 3;
            alpha[dx * 4 + 3] = alpha[dx * 4 + 2] + alpha[dx * 4 + 3];
            alpha[dx * 4 + 2] = alpha[dx * 4 + 1];
            alpha[dx * 4 + 1] = alpha[dx * 4 + 0];
            alpha[dx * 4 + 0] = 0.f;
        }
        if (sx >= w - 1) {
            sx = w - 3;
            alpha[dx * 4 + 3] = 1.f - alpha[dx * 4 + 0];
            alpha[dx * 4 + 2] = alpha[dx * 4 + 0];
            alpha[dx * 4 + 1] = 0.f;
            alpha[dx * 4 + 0] = 0.f;
        }

        xofs[dx] = sx;
    }
}

FUNCTION_IRAM static void
resize_bicubic_image(const tensor_t *src, tensor_t *dst, float *alpha, int *xofs, float *beta, int *yofs) {
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

typedef struct interp_nearest_context_t {
    upsample_config_t *interp;
    int h;
    int w;
    int ow;
    int8_t *ptr;
    int8_t *output_ptr;
} interp_nearest_context_t;

FUNCTION_IRAM void interp_nearest_thread(interp_nearest_context_t *context, int y) {
    const int in_y = Min((int) (y / context->interp->height_scale), (context->h - 1));
    for (int x = 0; x < context->ow; ++x) {
        const int in_x = Min((int) (x / context->interp->width_scale), (context->w - 1));
        context->output_ptr[context->ow * y + x] = context->ptr[in_y * context->w + in_x];
    }
}

typedef void (*resize_image)(const tensor_t *, tensor_t *, float *, int *, float *, int *);

typedef struct interp_other_context_t {
    tensor_t *bottom_tensor;
    tensor_t *top_tensor;
    int *xofs;
    int *yofs;
    float *alpha;
    float *beta;
    resize_image resize;
} interp_other_context_t;

FUNCTION_IRAM static void interp_other_thread(interp_other_context_t *context, int q) {
    const tensor_t src = tensor_d2(context->bottom_tensor, q);
    tensor_t dst = tensor_d2(context->top_tensor, q);

    context->resize(&src, &dst, context->alpha, context->xofs, context->beta, context->yofs);
}

typedef struct upsample_extend_t {
    operation_t operation;

    void *work_buffer;
    int *xofs;
    int *yofs;
    float *alpha;
    float *beta;
} upsample_extend_t;

static int upsamplle_setup(operation_t *operation, network_t *network, option_t *opt) {
    upsample_extend_t *interp_extend = (upsample_extend_t *) operation;
    upsample_t *interp = (upsample_t *) operation->base;
    const blob_info_t *bottom = network_get_input_blob_info(network, operation, 0);
    const blob_info_t *top = network_get_output_blob_info(network, operation, 0);

    int ow = top->shape[3];
    int oh = top->shape[2];
    int w = bottom->shape[3];
    int h = bottom->shape[2];

    if (interp->config.upsample_type == 2)// bilinear
    {
        int *buf = (int *) fast_malloc((ow + oh + ow * 2 + oh * 2) * sizeof(int));
        interp_extend->work_buffer = buf;

        interp_extend->xofs = buf;//new int[ow];
        interp_extend->yofs = buf + ow;//new int[oh];

        interp_extend->alpha = (float *) (buf + ow + oh);//new float[ow * 2];
        interp_extend->beta = (float *) (buf + ow + oh + ow * 2);//new float[oh * 2];

        linear_coeffs(w, ow, interp_extend->xofs, interp_extend->alpha);
        linear_coeffs(h, oh, interp_extend->yofs, interp_extend->beta);

        return CNET_STATUS_SUCCESS;
    } else if (interp->config.upsample_type == 3)// bicubic
    {
        int *buf = (int *) fast_malloc((ow + oh + ow * 4 + oh * 4) * sizeof(int));
        interp_extend->work_buffer = buf;

        interp_extend->xofs = buf;//new int[ow];
        interp_extend->yofs = buf + ow;//new int[oh];

        interp_extend->alpha = (float *) (buf + ow + oh);//new float[ow * 4];
        interp_extend->beta = (float *) (buf + ow + oh + ow * 4);//new float[oh * 4];

        cubic_coeffs(w, ow, interp_extend->xofs, interp_extend->alpha);
        cubic_coeffs(h, oh, interp_extend->yofs, interp_extend->beta);

        return CNET_STATUS_SUCCESS;
    }

    return CNET_STATUS_SUCCESS;
}

static int upsamplle_teardown(operation_t *operation, network_t *network, option_t *opt){
    upsample_extend_t *interp_extend = (upsample_extend_t *) operation;
    if(interp_extend->work_buffer != NULL){
        fast_free(interp_extend->work_buffer);
    }

    interp_extend->work_buffer = NULL;
    interp_extend->xofs = NULL;
    interp_extend->yofs = NULL;
    interp_extend->alpha = NULL;
    interp_extend->beta = NULL;

    return CNET_STATUS_SUCCESS;
}

static int upsample_forward_impl(
        operation_t *operation,
        tensor_t *bottom_tensor,
        tensor_t *top_tensor,
        option_t *opt) {

    upsample_t *interp = (upsample_t *) operation->base;
    upsample_extend_t *interp_extend = (upsample_extend_t *) operation;

    int w = bottom_tensor->d0;
    int h = bottom_tensor->d1;
    int c = bottom_tensor->d2;

    int oh = top_tensor->d1;
    int ow = top_tensor->d0;

    if (bottom_tensor->dims == 1) {
        for (int q = 0; q < c; ++q) {
            tensor_t top_blob_c = tensor_d2(top_tensor, q);
            float *ptr = ((float *) bottom_tensor->data + q);
            tensor_fill_f(&top_blob_c, *ptr);
        }
        return CNET_STATUS_SUCCESS;
    }

    if (interp->config.upsample_type == 1)// nearest
    {
        for (int q = 0; q < c; ++q) {
            int8_t *ptr = (int8_t *) tensor_d2(bottom_tensor, q).data;
            int8_t *output_ptr = tensor_d2(top_tensor, q).data;

            interp_nearest_context_t context = {
                    &interp->config,
                    h,
                    w,
                    ow,
                    ptr,
                    output_ptr
            };
            PARALLELIZE_1D(interp_nearest_thread, context, oh);
        }
        return CNET_STATUS_SUCCESS;

    } else if (interp->config.upsample_type == 2)// bilinear
    {

        interp_other_context_t context = {
                bottom_tensor,
                top_tensor,
                interp_extend->xofs,
                interp_extend->yofs,
                interp_extend->alpha,
                interp_extend->beta,
                resize_bilinear_image
        };
        PARALLELIZE_1D(interp_other_thread, context, c);

        return CNET_STATUS_SUCCESS;
    } else if (interp->config.upsample_type == 3)// bicubic
    {
        interp_other_context_t context = {
                bottom_tensor,
                top_tensor,
                interp_extend->xofs,
                interp_extend->yofs,
                interp_extend->alpha,
                interp_extend->beta,
                resize_bicubic_image
        };
        PARALLELIZE_1D(interp_other_thread, context, c);

        return CNET_STATUS_SUCCESS;
    } else {
        fprintf(stderr, "unsupported resize type %d %d %d\n", interp->config.upsample_type, oh, ow);
        return CNET_STATUS_FAILED;
    }
}

FUNCTION_IRAM static int upsample_forward(
        operation_t *operation,
        vector_blob_container_t *bottom_tensors,
        vector_blob_container_t *top_tensors,
        option_t *opt) {

    tensor_t *bottom_tensor = &bottom_tensors->data[0].data;
    tensor_t *top_tensor = &top_tensors->data[0].data;

    return upsample_forward_impl(operation, bottom_tensor, top_tensor, opt);
}

IMPL_OPERATION_CREATOR(upsample) {
    struct upsample_extend_t *upsample = (upsample_extend_t *) fast_malloc(sizeof(upsample_extend_t));
    if (NULL == upsample) {
        printf("malloc for upsample failed\n");
        return NULL;
    }

    operation_basic_info_setup(&upsample->operation);
    upsample->operation.forward = upsample_forward;
    upsample->operation.setup = upsamplle_setup;
    upsample->operation.teardown = upsamplle_teardown;
    upsample->work_buffer = NULL;
    upsample->xofs = NULL;
    upsample->yofs = NULL;
    upsample->alpha = NULL;
    upsample->beta = NULL;
    return (operation_ptr) upsample;
}
