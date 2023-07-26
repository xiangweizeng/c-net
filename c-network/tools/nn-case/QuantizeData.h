/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef NCNN_QUANTIZE_DATA_H
#define NCNN_QUANTIZE_DATA_H

#include <cstdio>
#include <unistd.h>
#include <getopt.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <cstdlib>
#include <algorithm>
#include <map>

#include "platform.h"
#include "net.h"
#include "cpu.h"
#include "benchmark.h"

class QuantizeData
{
public:
    QuantizeData(std::string layer_name, int num);

    int initial_blob_max(ncnn::Mat data);
    int initial_histogram_interval();
    int initial_histogram_value();

    int normalize_histogram();
    int update_histogram(ncnn::Mat data);

    static float compute_kl_divergence(const std::vector<float> &dist_a, const std::vector<float> &dist_b);
    static int threshold_distribution(const std::vector<float> &distribution, int target_bin=128);
    float get_data_blob_threshold();

public:
    std::string name;

    float max_value;
    int num_bins;
    float histogram_interval;
    std::vector<float> histogram;

    float threshold;
    int threshold_bin;
    float scale;
};

#endif //NCNN_QUANTIZE_DATA_H
