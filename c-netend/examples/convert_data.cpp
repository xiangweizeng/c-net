/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "net.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cstdio>

static inline int16_t float2int8(float v) {
    int int32 = round(v);
    if (int32 > 32767) return 32767;
    if (int32 < -32768) return -32768;
    return (int16_t)int32;
}


int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s [image_path] [cpp_path]\n", argv[0]);
        return -1;
    }

    const char* image_path = argv[1];
    const char* cpp_path = argv[2];

    cv::Mat bgr = cv::imread(image_path, 1);
    if (bgr.empty())
    {
        fprintf(stderr, "cv::imread %s failed\n", image_path);
        return -1;
    }

    FILE* cpp = fopen(cpp_path, "wb");
    if (cpp == nullptr) {
        fprintf(stderr, "open file %s failed\n", cpp_path);
        return -1;
    }

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR, bgr.cols, bgr.rows, 320, 240);
    float scaled = 218.225388;
    const float mean_vals[3] = { 104.f, 117.f, 123.f };
    const float norm_Vals[3] = { scaled, scaled, scaled };
    in.substract_mean_normalize(mean_vals, norm_Vals);

    fprintf(cpp, "\n__attribute__((aligned(4))) \n");
    fprintf(cpp, "static short image_data[] = {\n");

    int count = 0;
    for (int c = 0; c < in.c; c++) {
        ncnn::Mat channel = in.channel(c);
        size_t size = channel.total();
        float* data = channel;
        for (int i = 0; i < size; i++) {
            fprintf(cpp, "%d,", float2int8(data[i]));

            if ((count + 1) % 32 == 0) {
                fprintf(cpp, "\n");
            }

            count++;
        }
    }

    fprintf(cpp, "};\n");
    fclose(cpp);
}