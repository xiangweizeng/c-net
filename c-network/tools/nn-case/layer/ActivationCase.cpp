/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include <layer/relu.h>
#include <layer/exp.h>
#include <layer/hardsigmoid.h>
#include <layer/hardswish.h>
#include <layer/log.h>
#include <layer/power.h>
#include <layer/threshold.h>
#include <layer/selu.h>
#include <layer/elu.h>
#include <layer/clip.h>
#include <layer/swish.h>
#include "ActivationCase.h"

ActivationCase::ActivationCase(CNetCase *network, ncnn::Layer *layer)
        : CNetLayerCase(network, layer) {}

bool ActivationCase::ignore() {
    return false;
}

bool ActivationCase::quantize_weights() {
    return true;
}

bool ActivationCase::get_layer_define(std::string &layer_define) {

    char buffer[1024] = {0};
    int ret = false;

    if(layer->type == "BNLL"){
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d);\n",
                layer->name.c_str(), bnll_activate_type);
        ret = true;
    }

    if(layer->type == "Clip"){
        ncnn::Clip *ac = dynamic_cast<ncnn::Clip *>(layer);
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d, %f, %f);\n",
                layer->name.c_str(), clip_activate_type, ac->min, ac->max);
        ret = true;
    }

    if(layer->type == "ELU"){
        ncnn::ELU *ac = dynamic_cast<ncnn::ELU *>(layer);
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d, %f);\n",
                layer->name.c_str(), elu_activate_type, ac->alpha);
        ret = true;
    }

    if(layer->type == "Exp"){
        ncnn::Exp *exp = dynamic_cast<ncnn::Exp *>(layer);
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d, %f, %f, %f);\n",
                layer->name.c_str(), exp_activate_type, exp->base, exp->scale, exp->scale);
        ret = true;
    }

    if(layer->type == "HardSigmoid"){
        ncnn::HardSigmoid *ac = dynamic_cast<ncnn::HardSigmoid *>(layer);
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d, %f, %f);\n",
                layer->name.c_str(), hard_sigmoid_activate_type, ac->alpha, ac->beta);
        ret = true;
    }

    if(layer->type == "HardSwish"){
        ncnn::HardSwish *ac = dynamic_cast<ncnn::HardSwish *>(layer);
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d, %f, %f);\n",
                layer->name.c_str(), hard_swish_activate_type, ac->alpha, ac->beta);
        ret = true;
    }

    if(layer->type == "Log"){
        ncnn::Log *ac = dynamic_cast<ncnn::Log *>(layer);
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d, %f, %f, %f);\n",
                layer->name.c_str(), log_activate_type, ac->base, ac->scale, ac->shift);
        ret = true;
    }

    if(layer->type == "Power"){
        ncnn::Power *ac = dynamic_cast<ncnn::Power *>(layer);
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d, %f, %f, %f);\n",
                layer->name.c_str(), power_activate_type, ac->power, ac->scale, ac->shift);
        ret = true;
    }

    if(layer->type == "ReLU"){
        ncnn::ReLU *relu = dynamic_cast<ncnn::ReLU *>(layer);
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d, %f);\n",
                layer->name.c_str(), relu_activate_type, relu->slope);
        ret = true;
    }

    if(layer->type == "Selu"){
        ncnn::SELU *ac = dynamic_cast<ncnn::SELU *>(layer);
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d, %f, %f);\n",
                layer->name.c_str(), selu_activate_type, ac->alpha, ac->lambda);
        ret = true;
    }

    if(layer->type == "Swish"){
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d);\n",
                layer->name.c_str(), swish_activate_type);
        ret = true;
    }

    if(layer->type == "Threshold"){
        ncnn::Threshold *ac = dynamic_cast<ncnn::Threshold *>(layer);
        sprintf(buffer, "DEFINE_ACTIVATION_LAYER(%s, %d, %f);\n",
                layer->name.c_str(), threshold_activate_type, ac->threshold);
        ret = true;
    }

    layer_define = buffer;
    return ret;
}

bool ActivationCase::get_layer_reference(std::string &reference) {
    reference = "REFERENCE_LAYER(activation, " + layer->name + ")";
    return true;
}

static PCNetLayerCase CaseCreator(CNetCase* network, ncnn::Layer *layer){
    return std::make_shared<ActivationCase>(network, layer);
}

REGISTER_CASE(BNLL, CaseCreator)
REGISTER_CASE(Clip, CaseCreator)
REGISTER_CASE(ELU, CaseCreator)
REGISTER_CASE(Exp, CaseCreator)
REGISTER_CASE(HardSigmoid, CaseCreator)
REGISTER_CASE(HardSwish, CaseCreator)
REGISTER_CASE(Log, CaseCreator)
REGISTER_CASE(Power, CaseCreator)
REGISTER_CASE(ReLU, CaseCreator)
REGISTER_CASE(Selu, CaseCreator)
REGISTER_CASE(Swish, CaseCreator)
REGISTER_CASE(Threshold, CaseCreator)