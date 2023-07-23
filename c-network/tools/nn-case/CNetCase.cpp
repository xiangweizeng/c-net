/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include <layer/memorydata.h>
#include <layer/permute.h>
#include <layer/reshape.h>
#include "CNetCase.h"
#include "QuantizeData.h"
#include "CNetLayerCase.h"

static std::string get_file_name(const std::string& path) {
    size_t pos = path.find_last_of('/');
    pos = pos >= 0 ? pos + 1 : 0;
    return path.substr(pos);
}

static void sanitize_name(std::string& name)
{
    for (char & i : name)
    {
        if (!isalnum(i))
        {
            i = '_';
        }
    }
}

static std::string string_upper(std::string in_string){
    for(size_t i = 0; i < in_string.length(); i++){
        in_string[i] = toupper(in_string[i]);
    }
    return in_string;
}

void write_weights_data(ncnn::Mat weights, FILE* cpp, const std::string& param_var)
{
    fprintf(cpp, "\n__attribute__((aligned(4))) \n");

    size_t size = weights.total() * weights.elemsize;
    if(size > 0){
        fprintf(cpp, "static const unsigned char %s_data[] = {\n", param_var.c_str());
        unsigned char* data = weights;
        for (size_t i = 0; i < size; i++) {
            fprintf(cpp, "0x%02x,", data[i]);

            if ((i + 1) % 32 == 0) {
                fprintf(cpp, "\n");
            }
        }
        fprintf(cpp, "};\n");
    } else{
        fprintf(cpp, "static const unsigned char* %s_data[] = NULL;\n", param_var.c_str());
    }

    fprintf(cpp, "DEFINE_TENSOR(%s, %zu, %s_data, %f)", param_var.c_str(), size, param_var.c_str(), 1.f);
}

CNetCase::CNetCase(data_type_t data_type)
        : data_type(data_type),
          blobs(mutable_blobs()),
          layers(mutable_layers()),
          quantize(CNetQuantizer::makeQuantizer(data_type)) {
    opt.lightmode = false;
}

bool CNetCase::setup_case_layers() {

    for(auto l : layers){
        sanitize_name(l->name);
        auto cl = CNetLayerCaseFactory::create_layer_case(this, l);
        if(!cl){
            printf("Layer: %s is not supported\n", l->type.c_str());
            return false;
        }
        case_layers[l] = cl;

        if(cl->ignore()){
            printf("Ignore :%s-%s\n", l->type.c_str(), l->name.c_str());
        }
    }

    return true;
}

bool CNetCase::prepare_blobs()
{
    for (auto layer : layers) {
        if (case_layers[layer]->ignore()) {
            continue;
        }

        for (size_t o = 0; o < layer->tops.size(); o++) {
            get_blob_output_name(layer, (int)o, true);
        }
    }

    for (auto layer : layers) {
        if (case_layers[layer]->ignore()) {
            continue;
        }

        for (size_t o = 0; o < layer->bottoms.size(); o++) {
            get_blob_input_name(layer, (int)o, true);
        }
    }

    return true;
}


bool CNetCase::load_blob_scales(const char* filename)
{
    blob_scale_table.clear();

    FILE* fp = fopen(filename, "rb");
    if (!fp)
    {
        fprintf(stderr, "Open %s failed.\n", filename);
        return false;
    }

    std::string key_str;
    std::vector<float> scales;

    std::vector<char> line(102400);
    char* pch;

    while (nullptr != std::fgets(line.data(), static_cast<int>(line.size()), fp))
    {
        float scale = 1.f;
        char key[256];
        line[strcspn(line.data(), "\r\n")] = 0;

        pch = strtok(line.data(), " ");

        if (pch == nullptr) break;

        bool is_key = true;
        while (pch != nullptr)
        {
            if (is_key)
            {
                sscanf(pch, "%255s", key);

                key_str = key;
                is_key = false;
            }
            else
            {
                sscanf(pch, "%f", &scale);
                scales.push_back(scale);
            }

            pch = strtok(nullptr, " ");
        }

        blob_scale_table[key_str] = scales[0];

        key_str.clear();
        scales.clear();
    }

    fclose(fp);

    return true;
}

bool CNetCase::fix_blob_scales() {
    for (auto layer : layers) {
       if(layer->type == "MemoryData"){
           auto *md = dynamic_cast<ncnn::MemoryData *>(layer);
           blob_scale_table[blobs[md->tops[0]].name] = quantize->get_scaled(md->data);
       }
    }

    return true;
}

bool CNetCase::transform(const char* hpp_path, const char* cpp_path) {

    std::string network_name = get_file_name(cpp_path);
    size_t length = network_name.find_last_of('.');
    network_name = network_name.substr(0, length > 0 ? length : network_name.length());
    sanitize_name(network_name);

    /// cpp file
    if(!generate_cpp_file(hpp_path, cpp_path, network_name)){
        return false;
    }


    /// header file
    generate_hpp_file(hpp_path, network_name);

    return true;
}

std::string CNetCase::get_blob_input_name(const ncnn::Layer* layer, int input, bool used)
{

    if (layers[blobs[layer->bottoms[input]].producer]->type == "Split") {
        return get_blob_input_name(layers[blobs[layer->bottoms[input]].producer], 0, used);
    }
    else {

        if(layers[blobs[layer->bottoms[input]].producer]->type == "Permute" && layer->type == "Reshape"){
            const auto* permute = dynamic_cast<const ncnn::Permute*>(layers[blobs[layer->bottoms[input]].producer]);
            const auto* reshape = dynamic_cast<const ncnn::Reshape*>(layer);
            if(permute->order_type == 3 && reshape->ndim <= 2){
                return get_blob_input_name(layers[blobs[layer->bottoms[input]].producer], 0, used);
            }
        }

        auto blob = blobs[layer->bottoms[input]];
        std::string inputName = "blob_" + layers[blob.producer]->name + "_" + blob.name;
        sanitize_name(inputName);

        if (used) {
            auto iter = case_blobs.find(inputName);
            if (iter != case_blobs.end()) {
                iter->second.consumers_count += 1;
            }
            else {
                blob_info_t blob_info = {
                        .shape = {1, blob.shape.c, blob.shape.h, blob.shape.w},
                        .data_type = int16_data_type,
                        .consumers_count = 1,
                        .scale = blob_scale_table[blob.name]

                };
                case_blobs.emplace(inputName, blob_info);
                case_blob_names.emplace(inputName, blob.name);
                blob_case_names.emplace(blob.name, inputName);
            }
        }
        return inputName;
    }
}

std::string CNetCase::get_blob_output_name(const ncnn::Layer* layer, int output, bool used)
{
    std::string outputName = "blob_" + layer->name + "_" + blobs[layer->tops[output]].name;
    sanitize_name(outputName);
    auto blob = blobs[layer->tops[output]];

    if (used) {
        auto iter = case_blobs.find(outputName);
        const std::set<std::string> float_outputs = {
                "Softmax",
                "Sigmoid"
        };

        if (iter == case_blobs.end()) {
            iter->second.consumers_count += 1;
            data_type_t output_data_type = float_outputs.find(layer->type) == float_outputs.end()
                                           ? data_type: float_data_type;
            blob_info_t blob_info = {
                    .shape = {1, blob.shape.c, blob.shape.h, blob.shape.w},
                    .data_type = output_data_type,
                    .consumers_count = 0,
                    .scale = blob_scale_table[blob.name]
            };
            case_blobs.emplace(outputName, blob_info);
            case_blob_names.emplace(outputName, blob.name);
            blob_case_names.emplace(blob.name, outputName);
        }
    }

    return outputName;
}

bool CNetCase::generate_cpp_file(const char *hpp_path, const char *cpp_path, const std::string &network_name) {
    FILE* cpp = fopen(cpp_path, "wb");
    fprintf(cpp, "/***\n * Auto Generate by nn-case\n*/\n\n");
    fprintf(cpp, "#include \"%s\"\n", get_file_name(hpp_path).c_str());

    /// quantize weights data to cpp file
    if(!transform_weights(cpp)){
        return false;
    }

    /// operation case
    if(!transform_operations(cpp)){
        return false;
    }

    /// generate network info
    generate_network_info(cpp, network_name);
    fclose(cpp);

    return true;
}

bool CNetCase::transform_operations(FILE *cpp) {

    for (auto layer : layers) {
        auto cl = case_layers[layer];
        if (cl->ignore()) {
            continue;
        }

        std::string layer_define;
        if(!cl->get_layer_define(layer_define)){
            printf("Get layer define failed :%s-%s\n", layer->type.c_str(), layer->name.c_str());
            return false;
        }
        fprintf(cpp, "%s", layer_define.c_str());
    }

    return true;
}

bool CNetCase::transform_weights(FILE *cpp) {/// operation weights
    for (auto layer : layers) {
        auto cl = case_layers[layer];
        if (cl->ignore()) {
            continue;
        }

        if(!cl->quantize_weights()){
            printf("Get layer weights failed :%s-%s\n", layer->type.c_str(), layer->name.c_str());
            return false;
        }
    }

    for (const auto& weight : quantize_weights) {
        write_weights_data(weight.second, cpp, weight.first);
    }

    return true;
}

void CNetCase::generate_network_info(FILE *cpp, const std::string &network_name) {

    /// blob info
    std::vector<std::string> sequence;
    fprintf(cpp, "\nstatic blob_info_t network_blobs[] = {\n");
    for (const auto& blob : case_blobs) {
        if (blob.second.consumers_count == 0) {
            get_run_sequence(case_blob_names[blob.first], sequence);
        }
        fprintf(cpp, "\tBLOB_INSTANCE(%d, %d, %d, %d, %d, %d, %f), \n",
                blob.second.shape[0],
                blob.second.shape[2],
                blob.second.shape[3],
                blob.second.shape[1],
                blob.second.data_type,
                blob.second.consumers_count,
                blob.second.scale);
    }
    fprintf(cpp, "};\n");

    /// layers array
    fprintf(cpp, "\n\n\n\nstatic operation_type_t* network_layers[] = {\n");
    std::vector<ncnn::Layer*> blob_layers;
    for (auto& layer_name : sequence) {
        auto index = find_layer_index_by_name(layer_name.c_str());
        auto cl = case_layers[layers[index]];
        if(cl->ignore()){
            continue;
        }

        std::string reference;
        cl->get_layer_reference(reference);

        fprintf(cpp, "\t%s,\n", reference.c_str());
        blob_layers.push_back(layers[index]);
    }
    fprintf(cpp, "};\n");

    for(auto layer : blob_layers){
        if(!layer->bottoms.empty()){
            fprintf(cpp, "static int blobs_input_%s_data [] = {\n", layer->name.c_str());
            for(size_t i = 0; i < layer->bottoms.size(); i++){
                std::string input = get_blob_input_name(layer, (int)i);
                fprintf(cpp, "%s_%s, ", network_name.c_str(), input.c_str());
            }
            fprintf(cpp, "\n};\n");
        } else{
            fprintf(cpp, "static int blobs_input_%s_data[] = {0};\n", layer->name.c_str());
        }

        fprintf(cpp, "static int blobs_output_%s_data [] = {\n", layer->name.c_str());
        for(size_t i = 0; i < layer->tops.size(); i++){
            std::string output = get_blob_output_name(layer, (int)i);
            fprintf(cpp, "%s_%s, ", network_name.c_str(), output.c_str());
        }
        fprintf(cpp, "\n};\n\n");
    }

    fprintf(cpp, "static operation_blobs_t blobs_operations_data [] = {\n");
    for(auto layer : blob_layers){
        fprintf(cpp, "\t{\n"
                     "\t  .input_count = %zu,\n"
                     "\t  .input_blobs = blobs_input_%s_data,\n"
                     "\t  .output_count = %zu,\n"
                     "\t  .output_blobs = blobs_output_%s_data,\n"
                     "\t},\n",
                layer->bottoms.size(),
                layer->name.c_str(),
                layer->tops.size(),
                layer->name.c_str());
    }
    fprintf(cpp, "\n};\n");

    /// network define
    fprintf(cpp,
            "\n\nnetwork_t network_%s = {                                                   \n"
            "   .blob_size = sizeof(network_blobs)/sizeof(blob_info_t),                     \n"
            "   .blobs = network_blobs,                                                     \n"
            "   .operation_size = sizeof(network_layers) / sizeof(operation_type_t*),       \n"
            "   .operations = network_layers,                                               \n"
            "   .operation_blobs = blobs_operations_data,                                   \n"
            "   .run_operations = NULL                                                      \n"
            "};",network_name.c_str());
}


void CNetCase::generate_hpp_file(const char* hpp_path, const std::string &network_name) {
    FILE* hpp = fopen(hpp_path, "wb");

    std::string guard_name = network_name;
    std::transform(guard_name.begin(), guard_name.end(), guard_name.begin(), toupper);

    fprintf(hpp, "/***\n * Auto Generate by esp - nn - case\n*/\n\n");
    fprintf(hpp, "\n#ifndef %s_HPP_ \n#define %s_HPP_\n\n", guard_name.c_str(), guard_name.c_str());
    fprintf(hpp, "#include \"network.h\"\n");
    fprintf(hpp, "#include \"operation_define.h\"\n\n");

    /// blob id define
    int blob_id_start = 0;
    for (const auto& blob : case_blobs) {
        fprintf(hpp, "#define %s_%s %d\n", network_name.c_str(), blob.first.c_str(), blob_id_start++);
    }

    /// blob id define
    fprintf(hpp, "\n#define %s_BLOB_SIZE %lu\n", string_upper(network_name).c_str(), case_blobs.size());
    fprintf(hpp, "\ntypedef struct %s_ncnn_blob{ const char *name; int blob_id;} %s_ncnn_blob_t;\n",
            network_name.c_str(), network_name.c_str());
    fprintf(hpp, "static %s_ncnn_blob_t %s_ncnn_blobs [] = {\n", network_name.c_str(), network_name.c_str());
    for (const auto& blob : blobs) {
        auto iter = blob_case_names.find((blob.name));
        if(iter != blob_case_names.end()){
            fprintf(hpp, "\t{\"%s\" , %s_%s},\n", blob.name.c_str(),  network_name.c_str(), iter->second.c_str());
        }
    }
    fprintf(hpp, "};\n\n");

    /// network
    fprintf(hpp, "extern network_t network_%s;\n", network_name.c_str());
    fprintf(hpp, "\n#endif//%s\n", guard_name.c_str());

    fclose(hpp);
}

void CNetCase::get_run_sequence(const std::string& blob_name, std::vector<std::string>& sequence)
{
    int index = find_blob_index_by_name(blob_name.c_str());
    const ncnn::Layer* layer = layers[blobs[index].producer];

    auto find = std::find(sequence.begin(), sequence.end(), layer->name);
    if (find != sequence.end()) {
        return;
    }

    if (layer->one_blob_only)
    {
        if (!layer->bottoms.empty()) {
            int bottom_blob_index = layer->bottoms[0];
            auto find2 = std::find(sequence.begin(), sequence.end(), layers[blobs[bottom_blob_index].producer]->name);
            if (find2 == sequence.end()) {
                get_run_sequence(blobs[bottom_blob_index].name, sequence);
            }
        }

        sequence.emplace_back(layer->name);
    }
    else
    {
        for (int bottom_blob_index : layer->bottoms)
        {
            auto find2 = std::find(sequence.begin(), sequence.end(), layers[blobs[bottom_blob_index].producer]->name);
            if (find2 == sequence.end()) {
                get_run_sequence(blobs[bottom_blob_index].name, sequence);
            }
        }

        sequence.emplace_back(layer->name);
    }
}
