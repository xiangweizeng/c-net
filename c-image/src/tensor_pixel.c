
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include <limits.h>
#include <math.h>
#include "tensor_pixel.h"

#undef MIN
#undef MAX
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

static tensor_t from_rgb(const unsigned char* rgb, int w, int h, allocator_t *allocator)
{
    tensor_t m = tensor_create_default();
    tensor_create_3d(&m, 3, w, h, 4u, allocator);
    if (tensor_empty(&m))
        return m;

    float* ptr0 = (float*)m.data;

    int size = w * h;
    int remain = size;
    for (; remain>0; remain--)
    {
        ptr0[0] = rgb[0];
        ptr0[1] = rgb[1];
        ptr0[2] = rgb[2];

        rgb += 3;
        ptr0 += 3;
    }

    return m;
}

static void to_rgb(tensor_t *m, unsigned char* rgb)
{
    const float* ptr0 = (const float*)m->data;

    int size = m->d0 * m->d1;

#define SATURATE_CAST_UCHAR(X) (unsigned char)MIN(MAX((int)(X), 0), 255);

    int remain = size;

    for (; remain>0; remain--)
    {
        rgb[0] = SATURATE_CAST_UCHAR(ptr0[0]);
        rgb[1] = SATURATE_CAST_UCHAR(ptr0[1]);
        rgb[2] = SATURATE_CAST_UCHAR(ptr0[2]);

        rgb += 3;
        ptr0+= 3;
    }

#undef SATURATE_CAST_UCHAR
}

static tensor_t from_gray(const unsigned char* gray, int w, int h, allocator_t *allocator)
{
    tensor_t m = tensor_create_default();
    tensor_create_3d(&m, 1, w, h, 4u, allocator);
    if (tensor_empty(&m))
        return m;

    float* ptr = (float*)m.data;

    int size = w * h;

    int remain = size;
    for (; remain>0; remain--)
    {
        *ptr = *gray;

        gray++;
        ptr++;
    }

    return m;
}

static void to_gray(tensor_t *m, unsigned char* gray)
{
    const float* ptr = (const float*)m->data;

    int size = m->d0 * m->d1;

#define SATURATE_CAST_UCHAR(X) (unsigned char)MIN(MAX((int)(X), 0), 255);

    int remain = size;

    for (; remain>0; remain--)
    {
        *gray = SATURATE_CAST_UCHAR(*ptr);

        gray++;
        ptr++;
    }

#undef SATURATE_CAST_UCHAR
}

static tensor_t from_rgba(const unsigned char* rgba, int w, int h, allocator_t *allocator)
{
    tensor_t m = tensor_create_default();
    tensor_create_3d(&m, 4, w, h, 4u, allocator);
    if (tensor_empty(&m))
        return m;

    float* ptr0 = (float*)m.data;
    int size = w * h;


    int remain = size;
    for (; remain>0; remain--)
    {
        ptr0[0] = rgba[0];
        ptr0[1] = rgba[1];
        ptr0[2] = rgba[2];
        ptr0[3] = rgba[3];

        rgba += 4;
        ptr0 += 4;
    }

    return m;
}

static void to_rgba(tensor_t *m, unsigned char* rgba)
{
    const float* ptr0 = (const float*)m->data;

    int size = m->d0 * m->d1;

#define SATURATE_CAST_UCHAR(X) (unsigned char)MIN(MAX((int)(X), 0), 255);

    int remain = size;

    for (; remain>0; remain--)
    {
        rgba[0] = SATURATE_CAST_UCHAR(ptr0[0]);
        rgba[1] = SATURATE_CAST_UCHAR(ptr0[1]);
        rgba[2] = SATURATE_CAST_UCHAR(ptr0[2]);
        rgba[3] = SATURATE_CAST_UCHAR(ptr0[3]);

        rgba += 4;
        ptr0 += 4;
    }

#undef SATURATE_CAST_UCHAR
}

static tensor_t from_rgb2bgr(const unsigned char* rgb, int w, int h, allocator_t *allocator)
{
    tensor_t m = tensor_create_default();
    tensor_create_3d(&m, 3, w, h, 4u, allocator);
    if (tensor_empty(&m))
        return m;

    float* ptr0 = (float*)m.data;

    int size = w * h;
    int remain = size;
    for (; remain>0; remain--)
    {
        ptr0[0] = rgb[2];
        ptr0[1] = rgb[1];
        ptr0[2] = rgb[0];

        rgb += 3;
        ptr0 += 3;
    }

    return m;
}

static void to_bgr2rgb(tensor_t *m, unsigned char* rgb)
{
    const float* ptr0 = (const float*)m->data;

    int size = m->d0 * m->d1;

#define SATURATE_CAST_UCHAR(X) (unsigned char)MIN(MAX((int)(X), 0), 255);

    int remain = size;

    for (; remain>0; remain--)
    {
        rgb[2] = SATURATE_CAST_UCHAR(ptr0[0]);
        rgb[1] = SATURATE_CAST_UCHAR(ptr0[1]);
        rgb[0] = SATURATE_CAST_UCHAR(ptr0[2]);

        rgb += 3;
        ptr0 += 3;
    }

#undef SATURATE_CAST_UCHAR
}

static tensor_t from_rgb2gray(const unsigned char* rgb, int w, int h, allocator_t *allocator)
{
    // coeffs for r g b = 0.299f, 0.587f, 0.114f
    const unsigned char Y_shift = 8;//14
    const unsigned char R2Y = 77;
    const unsigned char G2Y = 150;
    const unsigned char B2Y = 29;

    tensor_t m = tensor_create_default();
    tensor_create_3d(&m, 1, w, h, 4u, allocator);
    if (tensor_empty(&m))
        return m;

    float* ptr = (float*)m.data;

    int size = w * h;
    int remain = size;
    for (; remain>0; remain--)
    {
        *ptr = (rgb[0] * R2Y + rgb[1] * G2Y + rgb[2] * B2Y) >> Y_shift;

        rgb += 3;
        ptr++;
    }

    return m;
}

static tensor_t from_bgr2gray(const unsigned char* bgr, int w, int h, allocator_t *allocator)
{
    // coeffs for r g b = 0.299f, 0.587f, 0.114f
    const unsigned char Y_shift = 8;//14
    const unsigned char R2Y = 77;
    const unsigned char G2Y = 150;
    const unsigned char B2Y = 29;

    tensor_t m = tensor_create_default();
    tensor_create_3d(&m, 1,  w, h, 4u, allocator);
    if (tensor_empty(&m))
        return m;

    float* ptr = (float*)m.data;

    int size = w * h;

    int remain = size;
    for (; remain>0; remain--)
    {
        *ptr = (bgr[2] * R2Y + bgr[1] * G2Y + bgr[0] * B2Y) >> Y_shift;

        bgr += 3;
        ptr++;
    }

    return m;
}

static tensor_t from_gray2rgb(const unsigned char* gray, int w, int h, allocator_t *allocator)
{
    tensor_t m = tensor_create_default();
    tensor_create_3d(&m, 3, w, h, 4u, allocator);
    if (tensor_empty(&m))
        return m;

    float* ptr0 = (float*)m.data;
    int size = w * h;

    int remain = size;
    for (; remain>0; remain--)
    {
        ptr0[0] = *gray;
        ptr0[1] = *gray;
        ptr0[2] = *gray;

        gray++;
        ptr0 += 3;
    }

    return m;
}

static tensor_t from_rgba2rgb(const unsigned char* rgba, int w, int h, allocator_t *allocator)
{
    tensor_t m = tensor_create_default();
    tensor_create_3d(&m, 3, w, h, 4u, allocator);
    if (tensor_empty(&m))
        return m;

    float* ptr0 = (float*)m.data;

    int size = w * h;
    int remain = size;
    for (; remain>0; remain--)
    {
        ptr0[0] = rgba[0];
        ptr0[1] = rgba[1];
        ptr0[2] = rgba[2];

        rgba += 4;
        ptr0 += 3;
    }

    return m;
}

static tensor_t from_rgba2bgr(const unsigned char* rgba, int w, int h, allocator_t *allocator)
{
    tensor_t m = tensor_create_default();
    tensor_create_3d(&m, 3, w, h, 4u, allocator);
    if (tensor_empty(&m))
        return m;

    float* ptr0 = (float*)m.data;
    int size = w * h;

    int remain = size;
    for (; remain>0; remain--)
    {
        ptr0[0] = rgba[2];
        ptr0[1] = rgba[1];
        ptr0[2] = rgba[0];

        rgba += 4;
        ptr0 += 3;
    }

    return m;
}

static tensor_t from_rgba2gray(const unsigned char* rgba, int w, int h, allocator_t *allocator)
{
    // coeffs for r g b = 0.299f, 0.587f, 0.114f
    const unsigned char Y_shift = 8;//14
    const unsigned char R2Y = 77;
    const unsigned char G2Y = 150;
    const unsigned char B2Y = 29;

    tensor_t m = tensor_create_default();
    tensor_create_3d(&m, 1, w, h, 4u, allocator);
    if (tensor_empty(&m))
        return m;

    float* ptr = (float*)m.data;

    int size = w * h;
    int remain = size;
    for (; remain>0; remain--)
    {
        *ptr = (rgba[0] * R2Y + rgba[1] * G2Y + rgba[2] * B2Y) >> Y_shift;

        rgba += 4;
        ptr++;
    }

    return m;
}

void yuv420sp2rgb(const unsigned char* yuv420sp, int w, int h, unsigned char* rgb, allocator_t *allocator)
{
    const unsigned char* yptr = yuv420sp;
    const unsigned char* vuptr = yuv420sp + w * h;

    for (int y=0; y<h; y+=2)
    {
        const unsigned char* yptr0 = yptr;
        const unsigned char* yptr1 = yptr + w;
        unsigned char* rgb0 = rgb;
        unsigned char* rgb1 = rgb + w*3;


        int remain = w;

#define SATURATE_CAST_UCHAR(X) (unsigned char)MIN(MAX((int)(X), 0), 255);
        for (; remain>0; remain-=2)
        {
            // R = 1.164 * yy + 1.596 * vv
            // G = 1.164 * yy - 0.813 * vv - 0.391 * uu
            // B = 1.164 * yy              + 2.018 * uu

            // R = Y + (1.370705 * (V-128))
            // G = Y - (0.698001 * (V-128)) - (0.337633 * (U-128))
            // B = Y + (1.732446 * (U-128))

            // R = ((Y << 6) + 87.72512 * (V-128)) >> 6
            // G = ((Y << 6) - 44.672064 * (V-128) - 21.608512 * (U-128)) >> 6
            // B = ((Y << 6) + 110.876544 * (U-128)) >> 6

            // R = ((Y << 6) + 90 * (V-128)) >> 6
            // G = ((Y << 6) - 46 * (V-128) - 22 * (U-128)) >> 6
            // B = ((Y << 6) + 113 * (U-128)) >> 6

            // R = (yy + 90 * vv) >> 6
            // G = (yy - 46 * vv - 22 * uu) >> 6
            // B = (yy + 113 * uu) >> 6

            int v = vuptr[0] - 128;
            int u = vuptr[1] - 128;

            int ruv = 90 * v;
            int guv = -46 * v + -22 * u;
            int buv = 113 * u;

            int y00 = yptr0[0] << 6;
            rgb0[0] = SATURATE_CAST_UCHAR((y00 + ruv) >> 6);
            rgb0[1] = SATURATE_CAST_UCHAR((y00 + guv) >> 6);
            rgb0[2] = SATURATE_CAST_UCHAR((y00 + buv) >> 6);

            int y01 = yptr0[1] << 6;
            rgb0[3] = SATURATE_CAST_UCHAR((y01 + ruv) >> 6);
            rgb0[4] = SATURATE_CAST_UCHAR((y01 + guv) >> 6);
            rgb0[5] = SATURATE_CAST_UCHAR((y01 + buv) >> 6);

            int y10 = yptr1[0] << 6;
            rgb1[0] = SATURATE_CAST_UCHAR((y10 + ruv) >> 6);
            rgb1[1] = SATURATE_CAST_UCHAR((y10 + guv) >> 6);
            rgb1[2] = SATURATE_CAST_UCHAR((y10 + buv) >> 6);

            int y11 = yptr1[1] << 6;
            rgb1[3] = SATURATE_CAST_UCHAR((y11 + ruv) >> 6);
            rgb1[4] = SATURATE_CAST_UCHAR((y11 + guv) >> 6);
            rgb1[5] = SATURATE_CAST_UCHAR((y11 + buv) >> 6);

            yptr0 += 2;
            yptr1 += 2;
            vuptr += 2;
            rgb0 += 6;
            rgb1 += 6;
        }
#undef SATURATE_CAST_UCHAR

        yptr += 2*w;
        rgb += 2*3*w;
    }
}


void resize_bilinear_c1(const unsigned char* src, int srcw, int srch, unsigned char* dst, int w, int h)
{
    const int INTER_RESIZE_COEF_BITS=11;
    const int INTER_RESIZE_COEF_SCALE=1 << INTER_RESIZE_COEF_BITS;
//     const int ONE=INTER_RESIZE_COEF_SCALE;

    double scale_x = (double)srcw / w;
    double scale_y = (double)srch / h;

    int* buf = (int*)fast_malloc((w + h + w + h) *sizeof(int));

    int* xofs = buf;//new int[w];
    int* yofs = buf + w;//new int[h];

    short* ialpha = (short*)(buf + w + h);//new short[w * 2];
    short* ibeta = (short*)(buf + w + h + w);//new short[h * 2];

    float fx;
    float fy;
    int sx;
    int sy;

#define SATURATE_CAST_SHORT(X) (short)MIN(MIN((int)(X + (X >= 0.f ? 0.5f : -0.5f)), SHRT_MIN), SHRT_MAX);

    for (int dx = 0; dx < w; dx++)
    {
        fx = (float)((dx + 0.5) * scale_x - 0.5);
        sx = floor(fx);
        fx -= sx;

        if (sx < 0)
        {
            sx = 0;
            fx = 0.f;
        }
        if (sx >= srcw - 1)
        {
            sx = srcw - 2;
            fx = 1.f;
        }

        xofs[dx] = sx;

        float a0 = (1.f - fx) * INTER_RESIZE_COEF_SCALE;
        float a1 =        fx  * INTER_RESIZE_COEF_SCALE;

        ialpha[dx*2    ] = SATURATE_CAST_SHORT(a0);
        ialpha[dx*2 + 1] = SATURATE_CAST_SHORT(a1);
    }

    for (int dy = 0; dy < h; dy++)
    {
        fy = (float)((dy + 0.5) * scale_y - 0.5);
        sy = floor(fy);
        fy -= sy;

        if (sy < 0)
        {
            sy = 0;
            fy = 0.f;
        }
        if (sy >= srch - 1)
        {
            sy = srch - 2;
            fy = 1.f;
        }

        yofs[dy] = sy;

        float b0 = (1.f - fy) * INTER_RESIZE_COEF_SCALE;
        float b1 =        fy  * INTER_RESIZE_COEF_SCALE;

        ibeta[dy*2    ] = SATURATE_CAST_SHORT(b0);
        ibeta[dy*2 + 1] = SATURATE_CAST_SHORT(b1);
    }

#undef SATURATE_CAST_SHORT

    // loop body
    tensor_t rowsbuf0 = tensor_create_default();
    tensor_create_1d(&rowsbuf0, w, (size_t)2u, allocator_create());
    tensor_t rowsbuf1 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w, (size_t)2u, allocator_create());
    short* rows0 = (short*)rowsbuf0.data;
    short* rows1 = (short*)rowsbuf1.data;

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
            short* rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const unsigned char *S1 = src + srcw * (sy+1);

            const short* ialphap = ialpha;
            short* rows1p = rows1;
            for ( int dx = 0; dx < w; dx++ )
            {
                int sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S1p = S1 + sx;
                rows1p[dx] = (S1p[0]*a0 + S1p[1]*a1) >> 4;

                ialphap += 2;
            }
        }
        else
        {
            // hresize two rows
            const unsigned char *S0 = src + srcw * (sy);
            const unsigned char *S1 = src + srcw * (sy+1);

            const short* ialphap = ialpha;
            short* rows0p = rows0;
            short* rows1p = rows1;
            for ( int dx = 0; dx < w; dx++ )
            {
                int sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S0p = S0 + sx;
                const unsigned char* S1p = S1 + sx;
                rows0p[dx] = (S0p[0]*a0 + S0p[1]*a1) >> 4;
                rows1p[dx] = (S1p[0]*a0 + S1p[1]*a1) >> 4;

                ialphap += 2;
            }
        }

        prev_sy1 = sy;

        // vresize
        short b0 = ibeta[0];
        short b1 = ibeta[1];

        short* rows0p = rows0;
        short* rows1p = rows1;
        unsigned char* Dp = dst + w * (dy);

        int nn = 0;
        int remain = w - (nn << 3);

        for ( ; remain; --remain )
        {
            //D[x] = (rows0[x]*b0 + rows1[x]*b1) >> INTER_RESIZE_COEF_BITS;
            *Dp++ = (unsigned char)(( (short)((b0 * (short)(*rows0p++)) >> 16) + (short)((b1 * (short)(*rows1p++)) >> 16) + 2)>>2);
        }

        ibeta += 2;
    }

    fast_free(buf);
}

void resize_bilinear_c2(const unsigned char* src, int srcw, int srch, unsigned char* dst, int w, int h)
{
    const int INTER_RESIZE_COEF_BITS=11;
    const int INTER_RESIZE_COEF_SCALE=1 << INTER_RESIZE_COEF_BITS;
//     const int ONE=INTER_RESIZE_COEF_SCALE;

    double scale_x = (double)srcw / w;
    double scale_y = (double)srch / h;

    int* buf = (int*)fast_malloc((w + h + w + h)* sizeof(int));

    int* xofs = buf;//new int[w];
    int* yofs = buf + w;//new int[h];

    short* ialpha = (short*)(buf + w + h);//new short[w * 2];
    short* ibeta = (short*)(buf + w + h + w);//new short[h * 2];

    float fx;
    float fy;
    int sx;
    int sy;

#define SATURATE_CAST_SHORT(X) (short)MIN(MAX((int)(X + (X >= 0.f ? 0.5f : -0.5f)), SHRT_MIN), SHRT_MAX);

    for (int dx = 0; dx < w; dx++)
    {
        fx = (float)((dx + 0.5) * scale_x - 0.5);
        sx = floor(fx);
        fx -= sx;

        if (sx < 0)
        {
            sx = 0;
            fx = 0.f;
        }
        if (sx >= srcw - 1)
        {
            sx = srcw - 2;
            fx = 1.f;
        }

        xofs[dx] = sx*2;

        float a0 = (1.f - fx) * INTER_RESIZE_COEF_SCALE;
        float a1 =        fx  * INTER_RESIZE_COEF_SCALE;

        ialpha[dx*2    ] = SATURATE_CAST_SHORT(a0);
        ialpha[dx*2 + 1] = SATURATE_CAST_SHORT(a1);
    }

    for (int dy = 0; dy < h; dy++)
    {
        fy = (float)((dy + 0.5) * scale_y - 0.5);
        sy = floor(fy);
        fy -= sy;

        if (sy < 0)
        {
            sy = 0;
            fy = 0.f;
        }
        if (sy >= srch - 1)
        {
            sy = srch - 2;
            fy = 1.f;
        }

        yofs[dy] = sy*2;

        float b0 = (1.f - fy) * INTER_RESIZE_COEF_SCALE;
        float b1 =        fy  * INTER_RESIZE_COEF_SCALE;

        ibeta[dy*2    ] = SATURATE_CAST_SHORT(b0);
        ibeta[dy*2 + 1] = SATURATE_CAST_SHORT(b1);
    }

#undef SATURATE_CAST_SHORT

    // loop body
    tensor_t rowsbuf0 = tensor_create_default();
    tensor_create_1d(&rowsbuf0, w*2+2, (size_t)2u, allocator_create());
    tensor_t rowsbuf1 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w*2+2, (size_t)2u, allocator_create());
    short* rows0 = (short*)rowsbuf0.data;
    short* rows1 = (short*)rowsbuf1.data;

    int prev_sy1 = -4;

    for (int dy = 0; dy < h; dy++ )
    {
        int sy = yofs[dy];

        if (sy == prev_sy1)
        {
            // reuse all rows
        }
        else if (sy == prev_sy1 + 2)
        {
            // hresize one row
            short* rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const unsigned char *S1 = src + srcw * (sy+2);

            const short* ialphap = ialpha;
            short* rows1p = rows1;
            for ( int dx = 0; dx < w; dx++ )
            {
                int sx = xofs[dx];

                const unsigned char* S1p = S1 + sx;

                short a0 = ialphap[0];
                short a1 = ialphap[1];

                rows1p[0] = (S1p[0]*a0 + S1p[2]*a1) >> 4;
                rows1p[1] = (S1p[1]*a0 + S1p[3]*a1) >> 4;

                ialphap += 2;
                rows1p += 2;
            }
        }
        else
        {
            // hresize two rows
            const unsigned char *S0 = src + srcw * (sy);
            const unsigned char *S1 = src + srcw * (sy+2);

            const short* ialphap = ialpha;
            short* rows0p = rows0;
            short* rows1p = rows1;
            for ( int dx = 0; dx < w; dx++ )
            {
                int sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S0p = S0 + sx;
                const unsigned char* S1p = S1 + sx;

                rows0p[0] = (S0p[0]*a0 + S0p[2]*a1) >> 4;
                rows0p[1] = (S0p[1]*a0 + S0p[3]*a1) >> 4;
                rows1p[0] = (S1p[0]*a0 + S1p[2]*a1) >> 4;
                rows1p[1] = (S1p[1]*a0 + S1p[3]*a1) >> 4;

                ialphap += 2;
                rows0p += 2;
                rows1p += 2;
            }
        }

        prev_sy1 = sy;

        // vresize
        short b0 = ibeta[0];
        short b1 = ibeta[1];

        short* rows0p = rows0;
        short* rows1p = rows1;
        unsigned char* Dp = dst + w * 2 * (dy);

        int nn = 0;
        int remain = (w * 2) - (nn << 3);

        for ( ; remain; --remain )
        {
//             D[x] = (rows0[x]*b0 + rows1[x]*b1) >> INTER_RESIZE_COEF_BITS;
            *Dp++ = (unsigned char)(( (short)((b0 * (short)(*rows0p++)) >> 16) + (short)((b1 * (short)(*rows1p++)) >> 16) + 2)>>2);
        }

        ibeta += 2;
    }

    fast_free(buf);
    tensor_release(&rowsbuf0);
    tensor_release(&rowsbuf1);
}

void resize_bilinear_c3(const unsigned char* src, int srcw, int srch, unsigned char* dst, int w, int h)
{
    const int INTER_RESIZE_COEF_BITS=11;
    const int INTER_RESIZE_COEF_SCALE=1 << INTER_RESIZE_COEF_BITS;

    double scale_x = (double)srcw / w;
    double scale_y = (double)srch / h;

    int* buf = (int*)fast_malloc((w + h + w + h)*sizeof(int));

    int* xofs = buf;//new int[w];
    int* yofs = buf + w;//new int[h];

    short* ialpha = (short*)(buf + w + h);//new short[w * 2];
    short* ibeta = (short*)(buf + w + h + w);//new short[h * 2];

    float fx;
    float fy;
    int sx;
    int sy;

#define SATURATE_CAST_SHORT(X) (short)MIN(MAX((int)(X + (X >= 0.f ? 0.5f : -0.5f)), SHRT_MIN), SHRT_MAX);

    for (int dx = 0; dx < w; dx++)
    {
        fx = (float)((dx + 0.5) * scale_x - 0.5);
        sx = floor(fx);
        fx -= sx;

        if (sx < 0)
        {
            sx = 0;
            fx = 0.f;
        }
        if (sx >= srcw - 1)
        {
            sx = srcw - 2;
            fx = 1.f;
        }

        xofs[dx] = sx*3;

        float a0 = (1.f - fx) * INTER_RESIZE_COEF_SCALE;
        float a1 =        fx  * INTER_RESIZE_COEF_SCALE;

        ialpha[dx*2    ] = SATURATE_CAST_SHORT(a0);
        ialpha[dx*2 + 1] = SATURATE_CAST_SHORT(a1);
    }

    for (int dy = 0; dy < h; dy++)
    {
        fy = (float)((dy + 0.5) * scale_y - 0.5);
        sy = floor(fy);
        fy -= sy;

        if (sy < 0)
        {
            sy = 0;
            fy = 0.f;
        }
        if (sy >= srch - 1)
        {
            sy = srch - 2;
            fy = 1.f;
        }

        yofs[dy] = sy*3;

        float b0 = (1.f - fy) * INTER_RESIZE_COEF_SCALE;
        float b1 =        fy  * INTER_RESIZE_COEF_SCALE;

        ibeta[dy*2    ] = SATURATE_CAST_SHORT(b0);
        ibeta[dy*2 + 1] = SATURATE_CAST_SHORT(b1);
    }

#undef SATURATE_CAST_SHORT

    // loop body
    tensor_t rowsbuf0 = tensor_create_default();
    tensor_create_1d(&rowsbuf0, w*3+1, (size_t)2u, allocator_create());
    tensor_t rowsbuf1 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w*3+1, (size_t)2u, allocator_create());
    short* rows0 = (short*)rowsbuf0.data;
    short* rows1 = (short*)rowsbuf1.data;

    int prev_sy1 = -6;

    for (int dy = 0; dy < h; dy++ )
    {
        int sy = yofs[dy];

        if (sy == prev_sy1 + 3)
        {
            // hresize one row
            short* rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const unsigned char *S1 = src + srcw * (sy+3);

            const short* ialphap = ialpha;
            short* rows1p = rows1;
            for ( int dx = 0; dx < w; dx++ )
            {
                int sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S1p = S1 + sx;

                rows1p[0] = (S1p[0]*a0 + S1p[3]*a1) >> 4;
                rows1p[1] = (S1p[1]*a0 + S1p[4]*a1) >> 4;
                rows1p[2] = (S1p[2]*a0 + S1p[5]*a1) >> 4;

                ialphap += 2;
                rows1p += 3;
            }
        }
        else
        {
            // hresize two rows
            const unsigned char *S0 = src + srcw * (sy);
            const unsigned char *S1 = src + srcw * (sy+3);

            const short* ialphap = ialpha;
            short* rows0p = rows0;
            short* rows1p = rows1;
            for ( int dx = 0; dx < w; dx++ )
            {
                int sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S0p = S0 + sx;
                const unsigned char* S1p = S1 + sx;

                rows0p[0] = (S0p[0]*a0 + S0p[3]*a1) >> 4;
                rows0p[1] = (S0p[1]*a0 + S0p[4]*a1) >> 4;
                rows0p[2] = (S0p[2]*a0 + S0p[5]*a1) >> 4;
                rows1p[0] = (S1p[0]*a0 + S1p[3]*a1) >> 4;
                rows1p[1] = (S1p[1]*a0 + S1p[4]*a1) >> 4;
                rows1p[2] = (S1p[2]*a0 + S1p[5]*a1) >> 4;

                ialphap += 2;
                rows0p += 3;
                rows1p += 3;
            }
        }

        prev_sy1 = sy;

        // vresize
        short b0 = ibeta[0];
        short b1 = ibeta[1];

        short* rows0p = rows0;
        short* rows1p = rows1;
        unsigned char* Dp = dst + w * 3 * (dy);

        int nn = 0;
        int remain = (w * 3) - (nn << 3);

        for ( ; remain; --remain )
        {
            //D[x] = (rows0[x]*b0 + rows1[x]*b1) >> INTER_RESIZE_COEF_BITS;
            *Dp++ = (unsigned char)(( (short)((b0 * (short)(*rows0p++)) >> 16) +
                    (short)((b1 * (short)(*rows1p++)) >> 16) + 2)>>2);
        }

        ibeta += 2;
    }

    fast_free(buf);
    tensor_release(&rowsbuf0);
    tensor_release(&rowsbuf1);
}

void resize_bilinear_c4(const unsigned char* src, int srcw, int srch, unsigned char* dst, int w, int h)
{
    const int INTER_RESIZE_COEF_BITS=11;
    const int INTER_RESIZE_COEF_SCALE=1 << INTER_RESIZE_COEF_BITS;
//     const int ONE=INTER_RESIZE_COEF_SCALE;

    double scale_x = (double)srcw / w;
    double scale_y = (double)srch / h;

    int* buf = (int*)fast_malloc((w + h + w + h)*sizeof(int));

    int* xofs = buf;//new int[w];
    int* yofs = buf + w;//new int[h];

    short* ialpha = (short*)(buf + w + h);//new short[w * 2];
    short* ibeta = (short*)(buf + w + h + w);//new short[h * 2];

    float fx;
    float fy;
    int sx;
    int sy;

#define SATURATE_CAST_SHORT(X) (short)MIN(MAX((int)(X + (X >= 0.f ? 0.5f : -0.5f)), SHRT_MIN), SHRT_MAX);

    for (int dx = 0; dx < w; dx++)
    {
        fx = (float)((dx + 0.5) * scale_x - 0.5);
        sx = floor(fx);
        fx -= sx;

        if (sx < 0)
        {
            sx = 0;
            fx = 0.f;
        }
        if (sx >= srcw - 1)
        {
            sx = srcw - 2;
            fx = 1.f;
        }

        xofs[dx] = sx*4;

        float a0 = (1.f - fx) * INTER_RESIZE_COEF_SCALE;
        float a1 =        fx  * INTER_RESIZE_COEF_SCALE;

        ialpha[dx*2    ] = SATURATE_CAST_SHORT(a0);
        ialpha[dx*2 + 1] = SATURATE_CAST_SHORT(a1);
    }

    for (int dy = 0; dy < h; dy++)
    {
        fy = (float)((dy + 0.5) * scale_y - 0.5);
        sy = floor(fy);
        fy -= sy;

        if (sy < 0)
        {
            sy = 0;
            fy = 0.f;
        }
        if (sy >= srch - 1)
        {
            sy = srch - 2;
            fy = 1.f;
        }

        yofs[dy] = sy*4;

        float b0 = (1.f - fy) * INTER_RESIZE_COEF_SCALE;
        float b1 =        fy  * INTER_RESIZE_COEF_SCALE;

        ibeta[dy*2    ] = SATURATE_CAST_SHORT(b0);
        ibeta[dy*2 + 1] = SATURATE_CAST_SHORT(b1);
    }

#undef SATURATE_CAST_SHORT

    // loop body
    tensor_t rowsbuf0 = tensor_create_default();
    tensor_create_1d(&rowsbuf0, w*4, (size_t)2u, allocator_create());
    tensor_t rowsbuf1 = tensor_create_default();
    tensor_create_1d(&rowsbuf1, w*4, (size_t)2u, allocator_create());
    short* rows0 = (short*)rowsbuf0.data;
    short* rows1 = (short*)rowsbuf1.data;

    int prev_sy1 = -8;

    for (int dy = 0; dy < h; dy++ )
    {
        int sy = yofs[dy];

        if (sy == prev_sy1)
        {
            // reuse all rows
        }
        else if (sy == prev_sy1 + 4)
        {
            // hresize one row
            short* rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const unsigned char *S1 = src + srcw * (sy+4);

            const short* ialphap = ialpha;
            short* rows1p = rows1;
            for ( int dx = 0; dx < w; dx++ )
            {
                int sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S1p = S1 + sx;

                rows1p[0] = (S1p[0]*a0 + S1p[4]*a1) >> 4;
                rows1p[1] = (S1p[1]*a0 + S1p[5]*a1) >> 4;
                rows1p[2] = (S1p[2]*a0 + S1p[6]*a1) >> 4;
                rows1p[3] = (S1p[3]*a0 + S1p[7]*a1) >> 4;

                ialphap += 2;
                rows1p += 4;
            }
        }
        else
        {
            // hresize two rows
            const unsigned char *S0 = src + srcw * (sy);
            const unsigned char *S1 = src + srcw * (sy+4);

            const short* ialphap = ialpha;
            short* rows0p = rows0;
            short* rows1p = rows1;
            for ( int dx = 0; dx < w; dx++ )
            {
                int sx = xofs[dx];
                short a0 = ialphap[0];
                short a1 = ialphap[1];

                const unsigned char* S0p = S0 + sx;
                const unsigned char* S1p = S1 + sx;
                rows0p[0] = (S0p[0]*a0 + S0p[4]*a1) >> 4;
                rows0p[1] = (S0p[1]*a0 + S0p[5]*a1) >> 4;
                rows0p[2] = (S0p[2]*a0 + S0p[6]*a1) >> 4;
                rows0p[3] = (S0p[3]*a0 + S0p[7]*a1) >> 4;
                rows1p[0] = (S1p[0]*a0 + S1p[4]*a1) >> 4;
                rows1p[1] = (S1p[1]*a0 + S1p[5]*a1) >> 4;
                rows1p[2] = (S1p[2]*a0 + S1p[6]*a1) >> 4;
                rows1p[3] = (S1p[3]*a0 + S1p[7]*a1) >> 4;

                ialphap += 2;
                rows0p += 4;
                rows1p += 4;
            }
        }

        prev_sy1 = sy;

        // vresize
        short b0 = ibeta[0];
        short b1 = ibeta[1];

        short* rows0p = rows0;
        short* rows1p = rows1;
        unsigned char* Dp = dst + w * 4 * (dy);

        int nn = 0;
        int remain = (w * 4) - (nn << 3);

        for ( ; remain; --remain )
        {
//             D[x] = (rows0[x]*b0 + rows1[x]*b1) >> INTER_RESIZE_COEF_BITS;
            *Dp++ = (unsigned char)(( (short)((b0 * (short)(*rows0p++)) >> 16) + (short)((b1 * (short)(*rows1p++)) >> 16) + 2)>>2);
        }

        ibeta += 2;
    }

    fast_free(buf);
    tensor_release(&rowsbuf0);
    tensor_release(&rowsbuf1);
}

void resize_bilinear_yuv420sp(const unsigned char* src, int srcw, int srch, unsigned char* dst, int w, int h)
{
    // assert srcw % 2 == 0
    // assert srch % 2 == 0
    // assert w % 2 == 0
    // assert h % 2 == 0

    const unsigned char* srcY = src;
    unsigned char* dstY = dst;
    resize_bilinear_c1(srcY, srcw, srch, dstY, w, h);

    const unsigned char* srcUV = src + srcw * srch;
    unsigned char* dstUV = dst + w * h;
    resize_bilinear_c2(srcUV, srcw / 2, srch / 2, dstUV, w / 2, h / 2);
}

tensor_t tensor_from_pixels(const unsigned char* pixels, int type, int w, int h, allocator_t *allocator)
{
    if (type & PIXEL_CONVERT_MASK)
    {
        if (type == PIXEL_RGB2BGR || type == PIXEL_BGR2RGB)
            return from_rgb2bgr(pixels, w, h, allocator);

        if (type == PIXEL_RGB2GRAY)
            return from_rgb2gray(pixels, w, h, allocator);

        if (type == PIXEL_BGR2GRAY)
            return from_bgr2gray(pixels, w, h, allocator);

        if (type == PIXEL_GRAY2RGB || type == PIXEL_GRAY2BGR)
            return from_gray2rgb(pixels, w, h, allocator);

        if (type == PIXEL_RGBA2RGB)
            return from_rgba2rgb(pixels, w, h, allocator);

        if (type == PIXEL_RGBA2BGR)
            return from_rgba2bgr(pixels, w, h, allocator);

        if (type == PIXEL_RGBA2GRAY)
            return from_rgba2gray(pixels, w, h, allocator);
    }
    else
    {
        if (type == PIXEL_RGB || type == PIXEL_BGR)
            return from_rgb(pixels, w, h, allocator);

        if (type == PIXEL_GRAY)
            return from_gray(pixels, w, h, allocator);

        if (type == PIXEL_RGBA)
            return from_rgba(pixels, w, h, allocator);
    }

    return tensor_create_default();
}

tensor_t tensor_from_pixels_resize(const unsigned char* pixels, int type, int w, int h, int target_width, int target_height, allocator_t *allocator)
{
    if (w == target_width && h == target_height)
        return tensor_from_pixels(pixels, type, w, h, allocator);

    tensor_t m = tensor_create_default();

    int type_from = type & PIXEL_FORMAT_MASK;

    if (type_from == PIXEL_RGB || type_from == PIXEL_BGR)
    {
        tensor_t dst = tensor_create_default();
        tensor_create_3d(&dst, 3u, target_width, target_height, 1u, allocator);

        resize_bilinear_c3(pixels, w, h, dst.data, target_width, target_height);
        m = tensor_from_pixels(dst.data, type, target_width, target_height, allocator);

        tensor_release(&dst);
    }
    else if (type_from == PIXEL_GRAY)
    {
        tensor_t dst = tensor_create_default();
        tensor_create_2d(&m, target_width, target_height, 1u, allocator);

        resize_bilinear_c1(pixels, w, h, dst.data, target_width, target_height);
        m = tensor_from_pixels(dst.data, type, target_width, target_height, allocator);

        tensor_release(&dst);
    }
    else if (type_from == PIXEL_RGBA)
    {
        tensor_t dst = tensor_create_default();
        tensor_create_2d(&dst, target_width, target_height, 4u, allocator);

        resize_bilinear_c4(pixels, w, h, dst.data, target_width, target_height);
        m = tensor_from_pixels(dst.data, type, target_width, target_height, allocator);

        tensor_release(&dst);
    }

    return m;
}

void tensor_to_pixels(tensor_t *tensor, unsigned char* pixels, int type)
{
    if (type & PIXEL_CONVERT_MASK)
    {
        if (type == PIXEL_RGB2BGR || type == PIXEL_BGR2RGB)
            return to_bgr2rgb(tensor, pixels);
    }
    else
    {
        if (type == PIXEL_RGB || type == PIXEL_BGR)
            return to_rgb(tensor, pixels);

        if (type == PIXEL_GRAY)
            return to_gray(tensor, pixels);

        if (type == PIXEL_RGBA)
            return to_rgba(tensor, pixels);
    }
}

void tensor_to_pixels_resize(tensor_t *tensor, unsigned char* pixels, int type, int target_width, int target_height, allocator_t *allocator)
{
    if (tensor->d0 == target_width && tensor->d1 == target_height)
        return tensor_to_pixels(tensor, pixels, type);

    int type_to = (type & PIXEL_CONVERT_MASK) ? (type >> PIXEL_CONVERT_SHIFT) : (type & PIXEL_FORMAT_MASK);

    if (type_to == PIXEL_RGB || type_to == PIXEL_BGR)
    {
        tensor_t src = tensor_create_default();
        tensor_create_2d(&src, target_width, target_height, 3u, allocator);

        tensor_to_pixels(tensor, src.data, type);
        resize_bilinear_c3(src.data, tensor->d0, tensor->d1, pixels, target_width, target_height);

        tensor_release(&src);
    }
    else if (type_to == PIXEL_GRAY)
    {
        tensor_t src = tensor_create_default();
        tensor_create_2d(&src, target_width, target_height, 1u, allocator);

        tensor_to_pixels(tensor, src.data,  type);
        resize_bilinear_c1(src.data, tensor->d0, tensor->d1, pixels, target_width, target_height);

        tensor_release(&src);
    }
    else if (type_to == PIXEL_RGBA)
    {
        tensor_t src = tensor_create_default();
        tensor_create_2d(&src, target_width, target_height, 4u, allocator);

        tensor_to_pixels(tensor, src.data,  type);

        resize_bilinear_c4(src.data, tensor->d0, tensor->d1, pixels, target_width, target_height);
        tensor_release(&src);
    }
}

void tensor_substract_mean_normalize(tensor_t *tensor, const float* mean_vals, const float* norm_vals)
{
    int size = tensor->d2 * tensor->d1;
    int channels = tensor->d0;
    if (mean_vals && !norm_vals)
    {
        /// only mean
        float* ptr = (float*)tensor->data;
        for(int i = 0 ; i < size; i++) {
            for (int q = 0; q < channels; q++) {
                ptr[q] -= mean_vals[q];
            }
            ptr += channels;
        }
    }
    else if (!mean_vals && norm_vals)
    {
        /// only norm
        float* ptr = (float*)tensor->data;
        for(int i = 0 ; i < size; i++) {
            for (int q = 0; q < channels; q++) {
                ptr[q] *= norm_vals[q];
            }
            ptr += channels;
        }
    }
    else if (mean_vals && norm_vals)
    {
        float* ptr = (float*)tensor->data;
        for(int i = 0 ; i < size; i++) {
            for (int q = 0; q < channels; q++) {
                ptr[q] = (ptr[q] - mean_vals[q]) * norm_vals[q];
            }
            ptr += channels;
        }
    }
}

void draw_rgb_image_box(tensor_t a, int x1, int y1, int x2, int y2, float r, float g,
                        float b) {
    // normalize_image(a);
    int i;
    if (x1 < 0)
        x1 = 0;
    if (x1 >= a.d0)
        x1 = a.d0 - 1;
    if (x2 < 0)
        x2 = 0;
    if (x2 >= a.d0)
        x2 = a.d0 - 1;

    if (y1 < 0)
        y1 = 0;
    if (y1 >= a.d1)
        y1 = a.d1 - 1;
    if (y2 < 0)
        y2 = 0;
    if (y2 >= a.d1)
        y2 = a.d1 - 1;

    for (i = x1; i <= x2; ++i) {
        ((float*)a.data)[i + y1 * a.d0 + 0 * a.d0 * a.d1] = r;
        ((float*)a.data)[i + y2 * a.d0 + 0 * a.d0 * a.d1] = r;

        ((float*)a.data)[i + y1 * a.d0 + 1 * a.d0 * a.d1] = g;
        ((float*)a.data)[i + y2 * a.d0 + 1 * a.d0 * a.d1] = g;

        ((float*)a.data)[i + y1 * a.d0 + 2 * a.d0 * a.d1] = b;
        ((float*)a.data)[i + y2 * a.d0 + 2 * a.d0 * a.d1] = b;
    }

    for (i = y1; i <= y2; ++i) {
        ((float*)a.data)[x1 + i * a.d0 + 0 * a.d0 * a.d1] = r;
        ((float*)a.data)[x2 + i * a.d0 + 0 * a.d0 * a.d1] = r;

        ((float*)a.data)[x1 + i * a.d0 + 1 * a.d0 * a.d1] = g;
        ((float*)a.data)[x2 + i * a.d0 + 1 * a.d0 * a.d1] = g;

        ((float*)a.data)[x1 + i * a.d0 + 2 * a.d0 * a.d1] = b;
        ((float*)a.data)[x2 + i * a.d0 + 2 * a.d0 * a.d1] = b;
    }
}

void draw_rgb_image_box_width(tensor_t a, int x1, int y1, int x2, int y2, int w, float r, float g, float b) {
    int i;
    for (i = 0; i < w; ++i) {
        draw_rgb_image_box(a, x1 + i, y1 + i, x2 - i, y2 - i, r, g, b);
    }
}

void draw_rgb_image_point_width(tensor_t a, int x1, int y1, int w, float r, float g, float b) {
    for (int x = x1; x < x1 + w; ++x) {
        for(int y = y1 ; y < y1 + w; y ++){
            ((float*)a.data)[x + y * a.d0 + 0 * a.d0 * a.d1] = r;
            ((float*)a.data)[x + y * a.d0 + 1 * a.d0 * a.d1] = g;
            ((float*)a.data)[x + y * a.d0 + 2 * a.d0 * a.d1] = b;
        }
    }
}