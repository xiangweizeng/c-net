
#include "utils.h"
#include "quantize_data.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

static ncnn::Option g_default_option;
static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
static ncnn::PoolAllocator g_workspace_pool_allocator;

struct PreParam {
    float mean[3];
    float norm[3];
    int width;
    int height;
    bool swapRB;
};

static int post_training_quantize(const std::vector<std::string> &image_list, const std::string &param_path,
                                  const std::string &bin_path, const std::string &table_path,
                                  struct PreParam &per_param) {
    size_t size = image_list.size();

    ncnn::Net net;
    net.opt = g_default_option;

    net.load_param(param_path.c_str());
    net.load_model(bin_path.c_str());

    float mean_vals[3];
    float norm_vals[3];

    int width = per_param.width;
    int height = per_param.height;
    bool swapRB = per_param.swapRB;

    mean_vals[0] = per_param.mean[0];
    mean_vals[1] = per_param.mean[1];
    mean_vals[2] = per_param.mean[2];

    norm_vals[0] = per_param.norm[0];
    norm_vals[1] = per_param.norm[1];
    norm_vals[2] = per_param.norm[2];

    g_blob_pool_allocator.clear();
    g_workspace_pool_allocator.clear();

    FILE *fp = fopen(table_path.c_str(), "w");

    /// initial quantization data
    std::vector<QuantizeData> quantize_datas;
    for (auto &i : net.mutable_blobs()) {
        std::string layer_name = i.name;

        QuantizeData quantize_data(layer_name, 2048);
        quantize_datas.push_back(quantize_data);
    }

    /// step 1 count the max value
    printf("====> Quantize the activation.\n");
    printf("    ====> step 1 : find the max value.\n");

    for (size_t i = 0; i < image_list.size(); i++) {
        std::string img_name = image_list[i];

        if ((i + 1) % 100 == 0) {
            fprintf(stderr, "          %d/%d\n", static_cast<int>(i + 1), static_cast<int>(size));
        }

        cv::Mat bgr = cv::imread(img_name, cv::IMREAD_COLOR);

        if (bgr.empty()) {
            fprintf(stderr, "cv::imread %s failed\n", img_name.c_str());
            return -1;
        }

        ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, swapRB ? ncnn::Mat::PIXEL_BGR2RGB : ncnn::Mat::PIXEL_BGR,
                                                     bgr.cols, bgr.rows, width, height);
        in.substract_mean_normalize(mean_vals, norm_vals);

        ncnn::Extractor ex = net.create_extractor();
        ex.set_light_mode(false);
        ex.input(net.mutable_blobs()[0].name.c_str(), in);

        for (size_t b = 0; b < net.mutable_blobs().size(); b++) {
            std::string blob_name = net.mutable_blobs()[b].name;

            ncnn::Mat out;
            ex.extract(blob_name.c_str(), out);
            quantize_datas[b].initial_blob_max(out);
        }
    }

    /// step 2 histogram_interval
    printf("    ====> step 2 : generate the histogram_interval.\n");
    for (size_t i = 0; i < net.mutable_blobs().size(); i++) {
        std::string blob_name = net.mutable_blobs()[i].name;
        quantize_datas[i].initial_histogram_interval();
        fprintf(stderr, "%-20s : max = %-15f interval = %-10f\n", quantize_datas[i].name.c_str(),
                quantize_datas[i].max_value, quantize_datas[i].histogram_interval);
    }

    /// step 3 histogram
    printf("    ====> step 3 : generate the histogram.\n");
    for (size_t i = 0; i < image_list.size(); i++) {
        std::string img_name = image_list[i];

        if ((i + 1) % 100 == 0)
            fprintf(stderr, "          %d/%d\n", (int) (i + 1), (int) size);
        cv::Mat bgr = cv::imread(img_name, cv::IMREAD_COLOR);

        if (bgr.empty()) {
            fprintf(stderr, "cv::imread %s failed\n", img_name.c_str());
            return -1;
        }

        ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, swapRB ? ncnn::Mat::PIXEL_BGR2RGB : ncnn::Mat::PIXEL_BGR,
                                                     bgr.cols, bgr.rows, width, height);
        in.substract_mean_normalize(mean_vals, norm_vals);

        ncnn::Extractor ex = net.create_extractor();
        ex.set_light_mode(false);
        ex.input(net.mutable_blobs()[0].name.c_str(), in);

        for (size_t b = 0; b < net.mutable_blobs().size(); b++) {
            std::string blob_name = net.mutable_blobs()[b].name;

            ncnn::Mat out;
            ex.extract(blob_name.c_str(), out);
            quantize_datas[b].update_histogram(out);
        }
    }

    /// step4 kld
    printf("    ====> step 4 : using kld to find the best threshold value.\n");
    for (size_t i = 0; i < net.mutable_blobs().size(); i++) {
        std::string blob_name = net.mutable_blobs()[i].name;
        fprintf(stderr, "%-20s ", blob_name.c_str());

        quantize_datas[i].get_data_blob_threshold();
        fprintf(stderr, "bin : %-8d "
                        "threshold : %-15f "
                        "interval : %-10f\n",
                quantize_datas[i].threshold_bin,
                quantize_datas[i].threshold,
                quantize_datas[i].histogram_interval);

        fprintf(fp, "%s %f\n", blob_name.c_str(), quantize_datas[i].threshold);
    }

    fclose(fp);
    printf("====> Save the calibration table done.\n");

    return 0;
}

void showUsage() {
    std::cout << "example: ./calibration --param=squeeze-net-fp32.param"
               " --bin=squeeze-net-fp32.bin "
               " --images=images/"
               " --output=squeeze-net.table"
               " --mean=104,117,123"
               " --norm=1,1,1"
               " --size=227,227"
               " --swapRB "
               " --thread=2"
            << std::endl;
}

static int find_all_value_in_string(const std::string &values_string, std::vector<float> &value) {
    std::vector<int> masks_pos;

    for (size_t i = 0; i < values_string.size(); i++) {
        if (',' == values_string[i]) {
            masks_pos.push_back(static_cast<int>(i));
        }
    }

    if (masks_pos.empty()) {
        fprintf(stderr, "ERROR: Cannot find any ',' in string, please check.\n");
        return -1;
    }

    if (2 != masks_pos.size()) {
        fprintf(stderr, "ERROR: Char ',' in fist of string, please check.\n");
        return -1;
    }

    if (masks_pos.front() == 0) {
        fprintf(stderr, "ERROR: Char ',' in fist of string, please check.\n");
        return -1;
    }

    if (masks_pos.back() == 0) {
        fprintf(stderr, "ERROR: Char ',' in last of string, please check.\n");
        return -1;
    }

    for (size_t i = 0; i < masks_pos.size(); i++) {
        if (i > 0) {
            if (!(masks_pos[i] - masks_pos[i - 1] > 1)) {
                fprintf(stderr, "ERROR: Neighbouring char ',' was found.\n");
                return -1;
            }
        }
    }

    const cv::String ch0_val_str = values_string.substr(0, masks_pos[0]);
    const cv::String ch1_val_str = values_string.substr(masks_pos[0] + 1, masks_pos[1] - masks_pos[0] - 1);
    const cv::String ch2_val_str = values_string.substr(masks_pos[1] + 1, values_string.size() - masks_pos[1] - 1);

    value.push_back(static_cast<float>(std::atof(std::string(ch0_val_str).c_str())));
    value.push_back(static_cast<float>(std::atof(std::string(ch1_val_str).c_str())));
    value.push_back(static_cast<float>(std::atof(std::string(ch2_val_str).c_str())));

    return 0;
}


#if CV_MAJOR_VERSION < 3
class NcnnQuantCommandLineParser : public cv::CommandLineParser
{
public:
    NcnnQuantCommandLineParser(int argc, const char* const argv[], const char* key_map)
        : cv::CommandLineParser(argc, argv, key_map)
    {
    }
    bool has(const std::string& keys)
    {
        return cv::CommandLineParser::has(keys);
    }
    void printMessage()
    {
        cv::CommandLineParser::printParams();
    }
};
#endif

int main(int argc, char **argv) {
    std::cout << "--- ncnn post training quantization tool --- " << __TIME__ << " " << __DATE__ << std::endl;

    const char *key_map = "{help h usage ? |   | print this message }"
                          "{param p        |   | path to ncnn.param file }"
                          "{bin b          |   | path to ncnn.bin file }"
                          "{images i       |   | path to calibration images folder }"
                          "{output o       |   | path to output calibration table file }"
                          "{mean m         |   | value of mean (mean value, default is 104.0,117.0,123.0) }"
                          "{norm n         |   | value of normalize (scale value, default is 1.0,1.0,1.0) }"
                          "{size s         |   | the size of input image(using the resize the original image,default is w=224,h=224) }"
                          "{swapRB c       |   | flag which indicates that swap first and last channels in 3-channel image is necessary }"
                          "{thread t       | 4 | count of processing threads }";

    cv::CommandLineParser parser(argc, argv, key_map);

    if (parser.has("help")) {
        parser.printMessage();
        showUsage();
        return 0;
    }

    if (!parser.has("param") || !parser.has("bin") || !parser.has("images") || !parser.has("output") ||
        !parser.has("mean") || !parser.has("norm")) {
        std::cout << "Inputs is does not include all needed param, pleas check..." << std::endl;
        parser.printMessage();
        showUsage();
        return 0;
    }

    const std::string image_folder_path = parser.get<cv::String>("images");
    const std::string ncnn_param_file_path = parser.get<cv::String>("param");
    const std::string ncnn_bin_file_path = parser.get<cv::String>("bin");
    const std::string saved_table_file_path = parser.get<cv::String>("output");

    /// check the input param
    if (image_folder_path.empty() || ncnn_param_file_path.empty() || ncnn_bin_file_path.empty() ||
        saved_table_file_path.empty()) {
        fprintf(stderr, "One or more path may be empty, please check and try again.\n");
        return 0;
    }

    const int num_threads = parser.get<int>("thread");

    struct PreParam pre_param{};
    pre_param.mean[0] = 104.f;
    pre_param.mean[1] = 117.f;
    pre_param.mean[2] = 103.f;
    pre_param.norm[0] = 1.f;
    pre_param.norm[1] = 1.f;
    pre_param.norm[2] = 1.f;
    pre_param.width = 224;
    pre_param.height = 224;
    pre_param.swapRB = false;

    if (parser.has("mean")) {
        const std::string mean_str = parser.get<std::string>("mean");

        std::vector<float> mean_values;
        const int ret = find_all_value_in_string(mean_str, mean_values);
        if (0 != ret && 3 != mean_values.size()) {
            fprintf(stderr, "ERROR: Searching mean value from --mean was failed.\n");

            return -1;
        }

        pre_param.mean[0] = mean_values[0];
        pre_param.mean[1] = mean_values[1];
        pre_param.mean[2] = mean_values[2];
    }

    if (parser.has("norm")) {
        const std::string norm_str = parser.get<std::string>("norm");

        std::vector<float> norm_values;
        const int ret = find_all_value_in_string(norm_str, norm_values);
        if (0 != ret && 3 != norm_values.size()) {
            fprintf(stderr, "ERROR: Searching mean value from --mean was failed, please check --mean param.\n");

            return -1;
        }

        pre_param.norm[0] = norm_values[0];
        pre_param.norm[1] = norm_values[1];
        pre_param.norm[2] = norm_values[2];
    }

    if (parser.has("size")) {
        cv::String size_str = parser.get<std::string>("size");

        size_t sep_pos = size_str.find_first_of(',');

        if (cv::String::npos != sep_pos && sep_pos < size_str.size()) {
            cv::String width_value_str;
            cv::String height_value_str;

            width_value_str = size_str.substr(0, sep_pos);
            height_value_str = size_str.substr(sep_pos + 1, size_str.size() - sep_pos - 1);

            pre_param.width = static_cast<int>(std::atoi(std::string(width_value_str).c_str()));
            pre_param.height = static_cast<int>(std::atoi(std::string(height_value_str).c_str()));
        } else {
            fprintf(stderr, "ERROR: Searching size value from --size was failed, please check --size param.\n");

            return -1;
        }
    }

    if (parser.has("swapRB")) {
        pre_param.swapRB = true;
    }

    g_blob_pool_allocator.set_size_compare_ratio(0.0f);
    g_workspace_pool_allocator.set_size_compare_ratio(0.5f);

    /// default option
    g_default_option.lightmode = true;
    g_default_option.num_threads = num_threads;
    g_default_option.blob_allocator = &g_blob_pool_allocator;
    g_default_option.workspace_allocator = &g_workspace_pool_allocator;

    g_default_option.use_winograd_convolution = true;
    g_default_option.use_sgemm_convolution = true;
    g_default_option.use_int8_inference = true;
    g_default_option.use_fp16_packed = true;
    g_default_option.use_fp16_storage = true;
    g_default_option.use_fp16_arithmetic = true;
    g_default_option.use_int8_storage = true;
    g_default_option.use_int8_arithmetic = true;

    ncnn::set_cpu_powersave(2);
    ncnn::set_omp_dynamic(0);
    ncnn::set_omp_num_threads(num_threads);

    std::vector<std::string> image_file_path_list;

    /// parse the image file.
    image_file_path_list = get_all_files(image_folder_path.c_str(), "jpg", FILE_TYPE);

    /// get the calibration table file, and save it.
    const int ret = post_training_quantize(
            image_file_path_list,
            ncnn_param_file_path,
            ncnn_bin_file_path,
            saved_table_file_path,
            pre_param);

    if (!ret) {
        fprintf(stderr,
                "\nNCNN Calibration table create success, best wish for your inference has a low accuracy loss...n");
    }

    return 0;
}