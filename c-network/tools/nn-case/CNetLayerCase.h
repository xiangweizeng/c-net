/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_LAYER_CASE_H
#define CNET_LAYER_CASE_H

#include <memory>
#include <functional>
#include <map>
#include <operation_config.h>
#include <blob.h>
#include "layer.h"
#include "CNetQuantizer.h"

/**
 * CNetCase
 */
class CNetCase;
typedef std::shared_ptr<CNetCase> PCNetCase;

/**
 * CNetLayerCase
 */
class CNetLayerCase{
public:

    /// CNetLayerCase
    /// \param network
    /// \param layer
    CNetLayerCase(CNetCase* network, ncnn::Layer *layer);

    /// ~CNetLayerCase
    virtual ~CNetLayerCase();

    /// Ignore
    /// \return
    virtual bool ignore() = 0;

    /// Quantize weights
    /// \return
    virtual bool quantize_weights() = 0;

    /// Get layer layer_define
    /// \return
    virtual bool get_layer_define(std::string &layer_define) = 0;

    /// Get layer reference
    /// \return
    virtual bool get_layer_reference(std::string &reference) = 0;

public:

    /// Get layer input blob name
    /// \param input
    /// \return
    std::string get_blob_input_name(int input);

    /// Get layer output blob name
    /// \param output
    /// \return
    std::string get_blob_output_name(int output);

protected:

    /// network case
    CNetCase* network;

    /// layer
    ncnn::Layer *layer;

    /// data type
    data_type_t &data_type;

    /// blobs
    std::vector<ncnn::Blob>& blobs;

    /// layers
    std::vector<ncnn::Layer*>& layers;

    /// quantize
    std::shared_ptr<CNetQuantizer> &quantize;

    /// blob infos
    std::map<std::string, blob_info_t> &case_blobs;

    /// blob name
    std::map<std::string, std::string> &case_blob_names;

    /// blob scales
    std::map<std::string, float> &blob_scale_table;

    /// quantize weights
    std::map<std::string, ncnn::Mat> &quantize_data_weights;
};

/// Case pointer
typedef std::shared_ptr<CNetLayerCase> PCNetLayerCase;
/// Case creator
typedef std::function<PCNetLayerCase(CNetCase*, ncnn::Layer *)> LayerCaseCreator;

class CNetLayerCaseFactory {
public:
    /// Register
    class Register {
    public:
        Register(const std::string &name, LayerCaseCreator creator) {
            register_layer_case(name, creator);
        }
    };

    /// Create layer case
    static PCNetLayerCase create_layer_case(CNetCase* network, ncnn::Layer*layer);

protected:
    /// Register case
    static void register_layer_case(const std::string &name, LayerCaseCreator creator);

protected:
    typedef std::map<std::string, LayerCaseCreator> TLayerCaseCreators;
    static std::shared_ptr<TLayerCaseCreators> layerCaseCreators;
};

/// register layer case
#define  CASE_UNUSED   __attribute__((unused))
#define REGISTER_CASE(name, creator) CASE_UNUSED CNetLayerCaseFactory::Register g_##name##_Register(#name, creator);

/// Ignore layer case creator
/// \param network
/// \param layer
/// \return
PCNetLayerCase IgnoreLayerCaseCreator(CNetCase*  network, ncnn::Layer *layer);

/// Impl case
#define IMPL_CASE(name) static PCNetLayerCase CaseCreator(CNetCase* network, ncnn::Layer *layer){   \
                               return std::make_shared<name##Case>(network, layer);                 \
                        }                                                                           \
                        REGISTER_CASE(name, CaseCreator)

#endif //CNET_LAYER_CASE_H
