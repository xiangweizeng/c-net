
#include "quantize_data.h"
#include <cassert>
#include <cmath>
#include <utility>

QuantizeData::QuantizeData(std::string layer_name, int num)
{
    name = std::move(layer_name);
    max_value = 0.0;
    num_bins = num;
    histogram_interval = 0.0;
    histogram.resize(num_bins);
    initial_histogram_value();
}

int QuantizeData::initial_blob_max(ncnn::Mat data)
{
    int channel_num = data.c;
    int size = data.w * data.h;

    for (int q=0; q<channel_num; q++)
    {
        const float *data_n = data.channel(q);
        for(int i=0; i<size; i++)
        {
            max_value = std::max(max_value, std::fabs(data_n[i]));
        }
    }

    return 0;
}

int QuantizeData::initial_histogram_interval()
{
    histogram_interval = max_value / num_bins;

    return 0;
}

int QuantizeData::initial_histogram_value()
{
    for (float & i : histogram)
    {
        i = 0.00001;
    }

    return 0;
}

int QuantizeData::normalize_histogram()
{
    const int length = histogram.size();
    float sum = 0;

    for (int i=0; i<length; i++)
        sum += histogram[i];

    for (int i=0; i<length; i++)
        histogram[i] /= sum;

    return 0;
}

int QuantizeData::update_histogram(ncnn::Mat data)
{
    int channel_num = data.c;
    int size = data.w * data.h;

    for (int q=0; q<channel_num; q++)
    {
        const float *data_n = data.channel(q);
        for(int i=0; i<size; i++)
        {
            if (data_n[i] == 0)
                continue;

            int index = std::min(static_cast<int>(std::abs(data_n[i]) / histogram_interval), 2047);

            histogram[index]++;
        }
    }

    return 0;
}

float QuantizeData::compute_kl_divergence(const std::vector<float> &dist_a, const std::vector<float> &dist_b)
{
    const int length = dist_a.size();
    assert(dist_b.size() == length);
    float result = 0;

    for (int i=0; i<length; i++)
    {
        if (dist_a[i] != 0)
        {
            if (dist_b[i] == 0)
            {
                result += 1;
            }
            else
            {
                result += dist_a[i] * log(dist_a[i] / dist_b[i]);
            }
        }
    }

    return result;
}

int QuantizeData::threshold_distribution(const std::vector<float> &distribution, const int target_bin)
{
    int target_threshold = target_bin;
    float min_kl_divergence = 1000;
    const int length = distribution.size();

    std::vector<float> quantize_distribution(target_bin);

    float threshold_sum = 0;
    for (int threshold=target_bin; threshold<length; threshold++)
    {
        threshold_sum += distribution[threshold];
    }

    for (int threshold=target_bin; threshold<length; threshold++)
    {

        std::vector<float> t_distribution(distribution.begin(), distribution.begin()+threshold);

        t_distribution[threshold-1] += threshold_sum;
        threshold_sum -= distribution[threshold];

        // get P
        fill(quantize_distribution.begin(), quantize_distribution.end(), 0);

        const float num_per_bin = static_cast<float>(threshold) / target_bin;

        for (int i=0; i<target_bin; i++)
        {
            const float start = i * num_per_bin;
            const float end = start + num_per_bin;

            const int left_upper = ceil(start);
            if (left_upper > start)
            {
                const float left_scale = left_upper - start;
                quantize_distribution[i] += left_scale * distribution[left_upper - 1];
            }

            const int right_lower = floor(end);

            if (right_lower < end)
            {

                const float right_scale = end - right_lower;
                quantize_distribution[i] += right_scale * distribution[right_lower];
            }

            for (int j=left_upper; j<right_lower; j++)
            {
                quantize_distribution[i] += distribution[j];
            }
        }

        // get Q
        std::vector<float> expand_distribution(threshold, 0);

        for (int i=0; i<target_bin; i++)
        {
            const float start = i * num_per_bin;
            const float end = start + num_per_bin;

            float count = 0;

            const int left_upper = ceil(start);
            float left_scale = 0;
            if (left_upper > start)
            {
                left_scale = left_upper - start;
                if (distribution[left_upper - 1] != 0)
                {
                    count += left_scale;
                }
            }

            const int right_lower = floor(end);
            float right_scale = 0;
            if (right_lower < end)
            {
                right_scale = end - right_lower;
                if (distribution[right_lower] != 0)
                {
                    count += right_scale;
                }
            }

            for (int j=left_upper; j<right_lower; j++)
            {
                if (distribution[j] != 0)
                {
                    count++;
                }
            }

            const float expand_value = quantize_distribution[i] / count;

            if (left_upper > start)
            {
                if (distribution[left_upper - 1] != 0)
                {
                    expand_distribution[left_upper - 1] += expand_value * left_scale;
                }
            }
            if (right_lower < end)
            {
                if (distribution[right_lower] != 0)
                {
                    expand_distribution[right_lower] += expand_value * right_scale;
                }
            }
            for (int j=left_upper; j<right_lower; j++)
            {
                if (distribution[j] != 0)
                {
                    expand_distribution[j] += expand_value;
                }
            }
        }

        // kl
        float kl_divergence = compute_kl_divergence(t_distribution, expand_distribution);

        // the best num of bin
        if (kl_divergence < min_kl_divergence)
        {
            min_kl_divergence = kl_divergence;
            target_threshold = threshold;
        }
    }

    return target_threshold;
}

float QuantizeData::get_data_blob_scale()
{
    normalize_histogram();
    threshold_bin = threshold_distribution(histogram);
    threshold = (threshold_bin + 0.5) * histogram_interval;
    scale = 32767 / threshold;
    return scale;
}