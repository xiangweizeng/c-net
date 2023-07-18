/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <math.h>
#include <stdlib.h>
#include "tensor.h"
#include "tensor_pixel.h"

void get_affine_matrix_from_5p(float *src_5pts, const float *dst_5pts, float *M) {
    float src[10], dst[10];
    memcpy(src, src_5pts, sizeof(float) * 10);
    memcpy(dst, dst_5pts, sizeof(float) * 10);

    float ptmp[2];
    ptmp[0] = ptmp[1] = 0;
    for (int i = 0; i < 5; ++i) {
        ptmp[0] += src[i];
        ptmp[1] += src[5 + i];
    }
    ptmp[0] /= 5;
    ptmp[1] /= 5;

    for (int i = 0; i < 5; ++i) {
        src[i] -= ptmp[0];
        src[5 + i] -= ptmp[1];
        dst[i] -= ptmp[0];
        dst[5 + i] -= ptmp[1];
    }

    float dst_x = (dst[3] + dst[4] - dst[0] - dst[1]) / 2, dst_y = (dst[8] + dst[9] - dst[5] - dst[6]) / 2;
    float src_x = (src[3] + src[4] - src[0] - src[1]) / 2, src_y = (src[8] + src[9] - src[5] - src[6]) / 2;
    float theta = atan2(dst_x, dst_y) - atan2(src_x, src_y);

    float scale = sqrt(pow(dst_x, 2) + pow(dst_y, 2)) / sqrt(pow(src_x, 2) + pow(src_y, 2));
    float pts1[10];
    float pts0[2];
    float _a = sin(theta), _b = cos(theta);
    pts0[0] = pts0[1] = 0;
    for (int i = 0; i < 5; ++i) {
        pts1[i] = scale * (src[i] * _b + src[i + 5] * _a);
        pts1[i + 5] = scale * (-src[i] * _a + src[i + 5] * _b);
        pts0[0] += (dst[i] - pts1[i]);
        pts0[1] += (dst[i + 5] - pts1[i + 5]);
    }
    pts0[0] /= 5;
    pts0[1] /= 5;

    float sqloss = 0;
    for (int i = 0; i < 5; ++i) {
        sqloss += ((pts0[0] + pts1[i] - dst[i]) * (pts0[0] + pts1[i] - dst[i])
                   + (pts0[1] + pts1[i + 5] - dst[i + 5]) * (pts0[1] + pts1[i + 5] - dst[i + 5]));
    }

    float square_sum = 0;
    for (int i = 0; i < 10; ++i) {
        square_sum += src[i] * src[i];
    }
    for (int t = 0; t < 200; ++t) {
        _a = 0;
        _b = 0;
        for (int i = 0; i < 5; ++i) {
            _a += ((pts0[0] - dst[i]) * src[i + 5] - (pts0[1] - dst[i + 5]) * src[i]);
            _b += ((pts0[0] - dst[i]) * src[i] + (pts0[1] - dst[i + 5]) * src[i + 5]);
        }
        if (_b < 0) {
            _b = -_b;
            _a = -_a;
        }
        float _s = sqrt(_a * _a + _b * _b);
        _b /= _s;
        _a /= _s;

        for (int i = 0; i < 5; ++i) {
            pts1[i] = scale * (src[i] * _b + src[i + 5] * _a);
            pts1[i + 5] = scale * (-src[i] * _a + src[i + 5] * _b);
        }

        float _scale = 0;
        for (int i = 0; i < 5; ++i) {
            _scale += ((dst[i] - pts0[0]) * pts1[i] + (dst[i + 5] - pts0[1]) * pts1[i + 5]);
        }
        _scale /= (square_sum * scale);
        for (int i = 0; i < 10; ++i) {
            pts1[i] *= (_scale / scale);
        }
        scale = _scale;

        pts0[0] = pts0[1] = 0;
        for (int i = 0; i < 5; ++i) {
            pts0[0] += (dst[i] - pts1[i]);
            pts0[1] += (dst[i + 5] - pts1[i + 5]);
        }
        pts0[0] /= 5;
        pts0[1] /= 5;

        float _sqloss = 0;
        for (int i = 0; i < 5; ++i) {
            _sqloss += ((pts0[0] + pts1[i] - dst[i]) * (pts0[0] + pts1[i] - dst[i])
                        + (pts0[1] + pts1[i + 5] - dst[i + 5]) * (pts0[1] + pts1[i + 5] - dst[i + 5]));
        }
        if (abs(_sqloss - sqloss) < 1e-2) {
            break;
        }
        sqloss = _sqloss;
    }

    for (int i = 0; i < 5; ++i) {
        pts1[i] += (pts0[0] + ptmp[0]);
        pts1[i + 5] += (pts0[1] + ptmp[1]);
    }

    M[0] = _b * scale;
    M[1] = _a * scale;
    M[3] = -_a * scale;
    M[4] = _b * scale;
    M[2] = pts0[0] + ptmp[0] - scale * (ptmp[0] * _b + ptmp[1] * _a);
    M[5] = pts0[1] + ptmp[1] - scale * (-ptmp[0] * _a + ptmp[1] * _b);
}


void get_affine_matrix_from_4p(float *src_4pts, const float *dst_4pts, float *M) {
    float src[8], dst[8];
    memcpy(src, src_4pts, sizeof(float) * 8);
    memcpy(dst, dst_4pts, sizeof(float) * 8);

    float ptmp[2];
    ptmp[0] = ptmp[1] = 0;
    for (int i = 0; i < 4; ++i) {
        ptmp[0] += src[i];
        ptmp[1] += src[4 + i];
    }
    ptmp[0] /= 4;
    ptmp[1] /= 4;

    for (int i = 0; i < 4; ++i) {
        src[i] -= ptmp[0];
        src[4 + i] -= ptmp[1];
        dst[i] -= ptmp[0];
        dst[4 + i] -= ptmp[1];
    }

    float dst_x = (dst[2] + dst[3] - dst[0] - dst[1]) / 2, dst_y = (dst[7] + dst[6] - dst[4] - dst[5]) / 2;
    float src_x = (src[2] + src[3] - src[0] - src[1]) / 2, src_y = (src[7] + src[6] - src[4] - src[5]) / 2;
    float theta = atan2(dst_x, dst_y) - atan2(src_x, src_y);

    float scale = sqrt(pow(dst_x, 2) + pow(dst_y, 2)) / sqrt(pow(src_x, 2) + pow(src_y, 2));
    float pts1[8];
    float pts0[2];
    float _a = sin(theta), _b = cos(theta);
    pts0[0] = pts0[1] = 0;
    for (int i = 0; i < 4; ++i) {
        pts1[i] = scale * (src[i] * _b + src[i + 4] * _a);
        pts1[i + 4] = scale * (-src[i] * _a + src[i + 4] * _b);
        pts0[0] += (dst[i] - pts1[i]);
        pts0[1] += (dst[i + 4] - pts1[i + 4]);
    }
    pts0[0] /= 4;
    pts0[1] /= 4;

    float sqloss = 0;
    for (int i = 0; i < 4; ++i) {
        sqloss += ((pts0[0] + pts1[i] - dst[i]) * (pts0[0] + pts1[i] - dst[i])
                   + (pts0[1] + pts1[i + 4] - dst[i + 4]) * (pts0[1] + pts1[i + 4] - dst[i + 4]));
    }

    float square_sum = 0;
    for (int i = 0; i < 8; ++i) {
        square_sum += src[i] * src[i];
    }
    for (int t = 0; t < 200; ++t) {
        _a = 0;
        _b = 0;
        for (int i = 0; i < 4; ++i) {
            _a += ((pts0[0] - dst[i]) * src[i + 4] - (pts0[1] - dst[i + 4]) * src[i]);
            _b += ((pts0[0] - dst[i]) * src[i] + (pts0[1] - dst[i + 4]) * src[i + 4]);
        }
        if (_b < 0) {
            _b = -_b;
            _a = -_a;
        }
        float _s = sqrt(_a * _a + _b * _b);
        _b /= _s;
        _a /= _s;

        for (int i = 0; i < 4; ++i) {
            pts1[i] = scale * (src[i] * _b + src[i + 4] * _a);
            pts1[i + 4] = scale * (-src[i] * _a + src[i + 4] * _b);
        }

        float _scale = 0;
        for (int i = 0; i < 4; ++i) {
            _scale += ((dst[i] - pts0[0]) * pts1[i] + (dst[i + 4] - pts0[1]) * pts1[i + 4]);
        }
        _scale /= (square_sum * scale);
        for (int i = 0; i < 8; ++i) {
            pts1[i] *= (_scale / scale);
        }
        scale = _scale;

        pts0[0] = pts0[1] = 0;
        for (int i = 0; i < 4; ++i) {
            pts0[0] += (dst[i] - pts1[i]);
            pts0[1] += (dst[i + 4] - pts1[i + 4]);
        }
        pts0[0] /= 4;
        pts0[1] /= 4;

        float _sqloss = 0;
        for (int i = 0; i < 4; ++i) {
            _sqloss += ((pts0[0] + pts1[i] - dst[i]) * (pts0[0] + pts1[i] - dst[i])
                        + (pts0[1] + pts1[i + 4] - dst[i + 4]) * (pts0[1] + pts1[i + 4] - dst[i + 4]));
        }
        if (abs(_sqloss - sqloss) < 1e-2) {
            break;
        }
        sqloss = _sqloss;
    }

    for (int i = 0; i < 4; ++i) {
        pts1[i] += (pts0[0] + ptmp[0]);
        pts1[i + 4] += (pts0[1] + ptmp[1]);
    }

    M[0] = _b * scale;
    M[1] = _a * scale;
    M[3] = -_a * scale;
    M[4] = _b * scale;
    M[2] = pts0[0] + ptmp[0] - scale * (ptmp[0] * _b + ptmp[1] * _a);
    M[5] = pts0[1] + ptmp[1] - scale * (-ptmp[0] * _a + ptmp[1] * _b);
}

int guass_jordan(float m[3][5], float eps) {
    int h = 3, w = 5;
    for (int y = 0; y < h; y++) {
        int maxrow = y;
        for (int y2 = y + 1; y2 < h; y2++) {
            if (abs(m[y2][y]) > abs(m[maxrow][y])) {
                maxrow = y2;
            }
        }

        for (int i = 0; i < w; i++) {
            float temp = m[y][i];
            m[y][i] = m[maxrow][i];
            m[maxrow][i] = temp;
        }

        if (fabs(m[y][y]) < eps) {
            return CNET_STATUS_FAILED;
        }

        for (int y2 = y + 1; y2 < h; y2++) {
            float c = m[y2][y] / m[y][y];
            for (int x = y; x < w; x++) {
                m[y2][x] -= m[y][x] * c;
            }
        }
    }

    for (int y = h - 1; y > -1; y--) {
        float c = m[y][y];
        for (int y2 = 0; y2 < y; y2++) {
            for (int x = w - 1; x > y - 1; x--) {
                m[y2][x] -= m[y][x] * m[y2][y] / c;
            }
        }

        m[y][y] /= c;
        for (int x = h; x < w; x++) {
            m[y][x] /= c;
        }
    }

    return CNET_STATUS_SUCCESS;
}

static void get_affine_matrix2(float *src_4pts, const float *dst_4pts, float *M) {
    float q[4][2], p[4][2];
    memcpy(q, src_4pts, sizeof(float) * 8);
    memcpy(p, dst_4pts, sizeof(float) * 8);

    float c[3][2] = {0};
    for (int j = 0; j < 2; j++) {
        for (int k = 0; k < 3; k++) {
            for (int i = 0; i < 4; i++) {
                float q_value = 1;
                if (k < 2) {
                    q_value = q[i][k];
                }
                c[k][j] += p[i][j] * q_value;
            }
        }
    }


    float Q[3][3] = {0};
    for (int k = 0; k < 4; k++) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                float i_value = 1;
                if (i < 2) {
                    i_value = q[k][i];
                }

                float j_value = 1;
                if (j < 2) {
                    j_value = q[k][j];
                }
                Q[i][j] += i_value * j_value;
            }
        }
    }

    float TEMP_M[3][5] = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            TEMP_M[i][j] = Q[i][j];
        }

        for (int j = 3; j < 5; j++) {
            TEMP_M[i][j] = c[i][j - 3];
        }
    }

    guass_jordan(TEMP_M, 1e-10);

    M[0] = TEMP_M[0][3];
    M[1] = TEMP_M[0][4];
    M[2] = TEMP_M[2][3];
    M[3] = TEMP_M[1][3];
    M[4] = TEMP_M[1][4];
    M[5] = TEMP_M[2][4];
}

tensor_t warp_affine_matrix(tensor_t *src, float *M, int dst_w, int dst_h) {
    int src_w = src->d0;
    int src_h = src->d1;

    unsigned char *src_u = (unsigned char *) fast_malloc(src_w * src_h * 3);
    memset(src_u, 0, src_w * src_h * 3);
    unsigned char *dst_u = (unsigned char *) fast_malloc(dst_w * dst_h * 3);
    memset(dst_u, 0, dst_w * dst_h * 3);

    tensor_to_pixels(src, src_u, PIXEL_RGB);

    float m[6];
    for (int i = 0; i < 6; i++)
        m[i] = M[i];
    float D = m[0] * m[4] - m[1] * m[3];
    D = D != 0 ? 1. / D : 0;
    float A11 = m[4] * D, A22 = m[0] * D;
    m[0] = A11;
    m[1] *= -D;
    m[3] *= -D;
    m[4] = A22;
    float b1 = -m[0] * m[2] - m[1] * m[5];
    float b2 = -m[3] * m[2] - m[4] * m[5];
    m[2] = b1;
    m[5] = b2;

    for (int y = 0; y < dst_h; y++) {
        for (int x = 0; x < dst_w; x++) {
            float fx = m[0] * x + m[1] * y + m[2];
            float fy = m[3] * x + m[4] * y + m[5];

            int sy = (int) floor(fy);
            fy -= sy;
            if (sy < 0 || sy >= src_h) continue;

            short cbufy[2];
            cbufy[0] = (short) ((1.f - fy) * 2048);
            cbufy[1] = 2048 - cbufy[0];

            int sx = (int) floor(fx);
            fx -= sx;
            if (sx < 0 || sx >= src_w) continue;

            short cbufx[2];
            cbufx[0] = (short) ((1.f - fx) * 2048);
            cbufx[1] = 2048 - cbufx[0];

            if (sy == src_h - 1 || sx == src_w - 1)
                continue;
            for (int c = 0; c < 3; c++) {
                dst_u[3 * (y * dst_w + x) + c] =
                        (
                                src_u[3 * (sy * src_w + sx) + c] * cbufx[0] * cbufy[0] +
                                src_u[3 * ((sy + 1) * src_w + sx) + c] * cbufx[0] * cbufy[1] +
                                src_u[3 * (sy * src_w + sx + 1) + c] * cbufx[1] * cbufy[0] +
                                src_u[3 * ((sy + 1) * src_w + sx + 1) + c] * cbufx[1] * cbufy[1]
                        ) >> 22;
            }
        }
    }

    tensor_t dst = tensor_from_pixels(dst_u, PIXEL_RGB, dst_w, dst_h, src->allocator);
    fast_free(src_u);
    fast_free(dst_u);

    return dst;
}

