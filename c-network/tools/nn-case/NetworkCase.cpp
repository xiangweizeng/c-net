

/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "CNetCase.h"

int main(int argc, char** argv) {
    if (argc < 6) {
        fprintf(stderr, "usage: %s [in_param] [in_bin] [out_hpp] [out_cpp] [table_path]"
                        "\n [quantize_type 1=int8 other=int16] [per_channels_quantize 1=true, other=false]\n",
                argv[0]);
        return -1;
    }

    const char* in_param = argv[1];
    const char* in_bin = argv[2];
    const char* out_hpp = argv[3];
    const char* out_cpp = argv[4];
    const char* table_path = argv[5];
    data_type_t data_type = int16_data_type;
    bool per_channels_quantize = false;

    if(argc >= 7){
        if(atoi(argv[6]) == 1){
            data_type = int8_data_type;
        }
    }

    if(argc >= 8){
        if(atoi(argv[7]) == 1){
            per_channels_quantize = true;
        }
    }

    CNetCase network_case(data_type, per_channels_quantize);
    if(network_case.load_param(in_param)){
        fprintf(stderr, "load param failed\n");
        return -1;
    }

    if(network_case.load_model(in_bin)){
        fprintf(stderr, "load model failed\n");
        return -1;
    }

    if(!network_case.setup_case_layers()){
        fprintf(stderr, "setup case layers failed\n");
        return -1;
    }

    if (!network_case.load_blob_scales(table_path)) {
        fprintf(stderr, "load blob scales failed\n");
        return -1;
    }

    if (!network_case.fix_blob_scales()) {
        fprintf(stderr, "fix blob scales failed\n");
        return -1;
    }

    if(!network_case.prepare_blobs()){
        fprintf(stderr, "prepare blobs failed\n");
        return -1;
    }

    if(!network_case.transform(out_hpp, out_cpp)){
        fprintf(stderr, "transform model failed\n");
        return -1;
    }

    return 0;
}