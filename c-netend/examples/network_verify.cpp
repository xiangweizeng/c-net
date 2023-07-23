

/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include <opencv2/imgcodecs.hpp>
#include <net.h>

extern "C" {
#include "tensor_pixel.h"
#include "models/plate/lpr-opt.h"
#include "models/plate/lpc-opt.h"
#include "models/face/mobilefacenet-opt.h"
#include "models/plate/plate-opt.h"
}

static void print_mat(ncnn::Mat &mat)
{
    for (int c = 0; c < mat.c; c++)
    {
        float *ptr = mat.channel(c);
        for (int h = 0; h < mat.h; h++)
        {
            for (int w = 0; w < mat.w; w++)
            {
                printf("%.2f ", ptr[h * mat.w + w]);
            }
            printf("\n");
        }
        printf("\n");
    }
}


template<typename Type>
int network_verify(network_t *network,
                   const char *image_path,
                   const std::string &ncnn_param,
                   const std::string &ncnn_bin,
                   const std::string &input,
                   int input_index,
                   int input_w, int input_h,
                   const float *mean_values,
                   const float *norm_values,
                   int type,
                   int blob_size,
                   const Type *ncnn_blobs) {
    cv::Mat bgr = cv::imread(image_path, 1);
    if (bgr.empty())
    {
        fprintf(stderr, "cv::imread %s failed\n", image_path);
        return -1;
    }

    session_t session = session_create(network);
    if(CNET_STATUS_FAILED  == network_setup_run_operations(network, nullptr)){
        printf("lpr load bin failed\n");
        return CNET_STATUS_FAILED;
    }

    {
        session.option.light_mode = 0;
        tensor_t image = tensor_from_pixels_resize(
                bgr.data, type, bgr.cols, bgr.rows, input_w, input_h,  nullptr);

        tensor_substract_mean_normalize(&image, mean_values, norm_values);
        session_set_input(&session, input_index, image);
        tensor_release(&image);

        if(CNET_STATUS_SUCCESS != session_run(&session)){
            printf("session extract plate_blob failed\n");
            session_release(&session);
            return 0 ;
        }
    }

    ncnn::Net net;
    net.opt.lightmode = 0;
    net.load_param(ncnn_param.c_str());
    net.load_model(ncnn_bin.c_str());

    auto extractor = net.create_extractor();
    auto image = ncnn::Mat::from_pixels_resize(bgr.data, type, bgr.cols, bgr.rows, input_w, input_h);
    image.substract_mean_normalize(mean_values, norm_values);
    extractor.input(input.c_str(), image);

    for(int i = 0; i < blob_size; i++){
        auto *info = &ncnn_blobs[i];

        ncnn::Mat reference;
        if(extractor.extract(info->name, reference)){
            printf("Extractor extract %s failed\n", info->name);
            continue;
        }

        tensor_t dst = session_get_output(&session, info->blob_id);
        if(tensor_empty(&dst)){
            printf("Session get output %s failed\n", info->name);
            continue;
        }

        if(info->blob_id == lpc_opt_blob_prob_prob){
            tensor_print(&dst);
            print_mat(reference);
        }

        tensor_t chw_dst = tensor_hwc2chw(&dst, nullptr);
        for(int c = 0; c < chw_dst.d2; c++){
            tensor_t dc = tensor_d2(&chw_dst, c);
            ncnn::Mat rc = reference.channel(c);

            auto *dc_data = static_cast<float *>(dc.data);
            auto *rc_data = static_cast<float *>(rc.data);

            int size = tensor_total(&dc);
            float diff = 0;
            for(int s = 0 ; s < size; s ++){
                diff += fabs(dc_data[s] - rc_data[s]);
            }

            if( (diff / size) > 9.0){
                printf("error:  %s/%d: %d-%f\n", info->name, info->blob_id,  c, diff / size);
                tensor_print(&dc);
                print_mat(rc);

                tensor_release(&dst);
                tensor_release(&chw_dst);
                return -1;
            }
        }

        tensor_release(&dst);
        tensor_release(&chw_dst);
    }

    network_teardown_run_operations(network, NULL);

    return 0;
}

int verify_lpr(const char *image_path) {
    int input_index = lpr_opt_blob_input_data;
    int input_w = 128;
    int input_h = 32;
    const float mean_values[3] = {116.407, 133.722, 124.187};
    const float norm_values[3] = {1.f, 1.f, 1.f};
    int type = PIXEL_RGB2BGR;

    std::string ncnn_param = "models/plate/lpr-opt.param";
    std::string ncnn_bin = "models/plate/lpr-opt.bin";

    network_t *network = &network_lpr_opt;
    auto * ncnn_blobs = lpr_opt_ncnn_blobs;
    int blob_size = LPR_OPT_BLOB_SIZE;

    return network_verify(
            network,
            image_path,
            ncnn_param,
            ncnn_bin,
            "data",
            input_index,
            input_w,
            input_h,
            mean_values,
            norm_values,
            type,
            blob_size,
            ncnn_blobs);
}

int verify_lpc(const char *image_path) {
    int input_index = lpc_opt_blob_data_data;
    int input_w = 110;
    int input_h = 22;

    const float mean_values[3] = {89.9372, 81.1989, 73.6352};
    const float norm_values[3] = {1.f, 1.f, 1.f};
    int type = PIXEL_RGB2BGR;

    std::string ncnn_param = "models/plate/lpc-opt.param";
    std::string ncnn_bin = "models/plate/lpc-opt.bin";

    network_t *network = &network_lpc_opt;
    auto * ncnn_blobs = lpc_opt_ncnn_blobs;
    int blob_size = LPC_OPT_BLOB_SIZE;

    return network_verify(
            network,
            image_path,
            ncnn_param,
            ncnn_bin,
            "data",
            input_index,
            input_w,
            input_h,
            mean_values,
            norm_values,
            type,
            blob_size,
            ncnn_blobs);
}

int verify_plate(const char *image_path) {
    int input_index = plate_opt_blob_input0_input0;
    int input_w = 320;
    int input_h = 240;

    const float mean_values[3] = { 104.f, 117.f, 123.f};
    const float norm_values[3] = { 1.f, 1.f, 1.f};
    int type = PIXEL_RGB2BGR;

    std::string ncnn_param = "models/plate/plate-opt.param";
    std::string ncnn_bin = "models/plate/plate-opt.bin";

    network_t *network = &network_plate_opt;
    auto * ncnn_blobs = plate_opt_ncnn_blobs;
    int blob_size = PLATE_OPT_BLOB_SIZE;

    return network_verify(
            network,
            image_path,
            ncnn_param,
            ncnn_bin,
            "input0",
            input_index,
            input_w,
            input_h,
            mean_values,
            norm_values,
            type,
            blob_size,
            ncnn_blobs);
}

int verify_mobilefacenet(const char *image_path) {
    int input_index = mobilefacenet_opt_blob_data_data;
    int input_w = 112;
    int input_h = 112;

    const float mean_values[] = {0, 0, 0};
    const float norm_values[] = {1, 1, 1};
    int type = PIXEL_RGB;

    std::string ncnn_param = "models/face/mobilefacenet-opt.param";
    std::string ncnn_bin = "models/face/mobilefacenet-opt.bin";

    network_t *network = &network_mobilefacenet_opt;
    auto * ncnn_blobs = mobilefacenet_opt_ncnn_blobs;
    int blob_size = MOBILEFACENET_OPT_BLOB_SIZE;

    return network_verify(
            network,
            image_path,
            ncnn_param,
            ncnn_bin,
            "data",
            input_index,
            input_w,
            input_h,
            mean_values,
            norm_values,
            type,
            blob_size,
            ncnn_blobs);
}


int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s  [image_path]\n", argv[0]);
        return -1;
    }

    option_init(1);
    const char* image_path = argv[1];
    verify_lpc(image_path);
//    verify_lpr(image_path);
//    verify_plate(image_path);
//    verify_mobilefacenet(image_path);
    option_uninit();
    return 0;
}

