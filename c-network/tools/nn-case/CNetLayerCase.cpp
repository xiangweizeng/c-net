/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "CNetLayerCase.h"
#include "CNetCase.h"

CNetLayerCase::CNetLayerCase(CNetCase* network, ncnn::Layer *layer)
        : network(network),
          layer(layer),
          data_type(network->data_type),
          per_channel_quantize(network->per_channel_quantize),
          blobs(network->blobs),
          layers(network->layers),
          quantize(network->quantize),
          case_blobs(network->case_blobs),
          case_blob_names(network->case_blob_names),
          blob_scale_table(network->blob_scale_table),
          quantize_data_weights(network->quantize_weights){
}

CNetLayerCase::~CNetLayerCase() {
}

std::string CNetLayerCase::get_blob_input_name(int input) {
    return network->get_blob_input_name(layer, input);
}

std::string CNetLayerCase::get_blob_output_name(int output) {
    return network->get_blob_output_name(layer, output);
}

std::shared_ptr<CNetLayerCaseFactory::TLayerCaseCreators> CNetLayerCaseFactory::layerCaseCreators;

void CNetLayerCaseFactory::register_layer_case(const std::string &name, LayerCaseCreator creator) {

    if (nullptr == layerCaseCreators) {
        layerCaseCreators = std::make_shared<TLayerCaseCreators>();
    }

    layerCaseCreators->emplace(name, creator);
}

PCNetLayerCase CNetLayerCaseFactory::create_layer_case(CNetCase* network, ncnn::Layer *layer) {
    auto iter = layerCaseCreators->find(layer->type);
    if(iter != layerCaseCreators->end()){
        return iter->second(network, layer);
    }

    return nullptr;
}

/**
 * IgnoreLayerCase
 */
class IgnoreLayerCase : public CNetLayerCase{
public:
    IgnoreLayerCase(CNetCase* network, ncnn::Layer *layer)
            : CNetLayerCase(network, layer) {}

    bool ignore() override{
        return true;
    }

    bool quantize_weights() override{
        return true;
    }

    bool get_layer_define(std::string &layer_define) override{
        layer_define = "";
        return true;
    }

    bool get_layer_reference(std::string &reference) override{
        reference = "";
        return true;
    }
};

PCNetLayerCase IgnoreLayerCaseCreator(CNetCase* network, ncnn::Layer *layer){
    return std::make_shared<IgnoreLayerCase>(network, layer);
}