/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/


#ifndef CNET_C_NET_CASE_H
#define CNET_C_NET_CASE_H

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include <cfloat>
#include <sstream>

#include "layer_type.h"
#include "net.h"

#include "CNetQuantizer.h"

/// Case pointer
class CNetLayerCase;
typedef std::shared_ptr<CNetLayerCase> PCNetLayerCase;

/**
 * CNetCase
 */
class CNetCase : public ncnn::Net, std::enable_shared_from_this<CNetCase>{
public:
    /// CNetCase
    /// \param data_type
    explicit CNetCase(data_type_t data_type);

    /// Setup case layers
    /// \return
    bool setup_case_layers();

    /// Prepare blobs
    /// \return
    bool prepare_blobs();

    /// Load blob scales
    /// \param filename
    /// \return
    bool load_blob_scales(const char* filename);

    /// Fix blob scales, such as MemoryData Layer
    /// \return
    bool fix_blob_scales();

    /// Transform model
    /// \param hpp_path
    /// \param cpp_path
    /// \return
    bool transform(const char* hpp_path, const char* cpp_path);

protected:

    /// Get layer input blob name
    /// \param layer
    /// \param input
    /// \param used
    /// \return
    std::string get_blob_input_name(
            const ncnn::Layer* layer,
            int input,
            bool used = false);

    /// Get layer output blob name
    /// \param layer
    /// \param output
    /// \param used
    /// \return
    std::string get_blob_output_name(
            const ncnn::Layer* layer,
            int output,
            bool used = false);

    /// Generate hpp file
    /// \param hpp_path
    /// \param network_name
    void generate_hpp_file(
            const char* hpp_path,
            const std::string &network_name);

    /// Generate cpp file
    /// \param hpp_path
    /// \param cpp_path
    /// \param network_name
    bool generate_cpp_file(
            const char *hpp_path,
            const char *cpp_path,
            const std::string &network_name);

    /// Transform weights
    /// \param cpp
    bool transform_weights(FILE *cpp);

    /// Transform operations
    /// \param cpp
    /// \return
    bool transform_operations(FILE *cpp);

    /// Get network info
    /// \param cpp
    /// \param network_name
    void generate_network_info(FILE *cpp, const std::string &network_name);

    /// Get run sequence
    /// \param blob_name
    /// \param sequence
    void get_run_sequence(const std::string& blob_name, std::vector<std::string>& sequence);

protected:
    /// data type
    data_type_t data_type;

    /// blobs
    std::vector<ncnn::Blob>& blobs;

    /// layers
    std::vector<ncnn::Layer*>& layers;

    /// case layers
    std::map<ncnn::Layer*, PCNetLayerCase> case_layers;

    /// quantize
    std::shared_ptr<CNetQuantizer> quantize;

    /// blob infos
    std::map<std::string, blob_info_t> case_blobs;

    /// blob names
    std::map<std::string, std::string> case_blob_names;

    /// blob names
    std::map<std::string, std::string> blob_case_names;

    /// blob scales
    std::map<std::string, float> blob_scale_table;

    /// quantize weights
    std::map<std::string, ncnn::Mat> quantize_weights;

protected:
    friend class CNetLayerCase;
    #include "layer/FriendLayerCase.ini"
};


#endif //CNET_C_NET_CASE_H
