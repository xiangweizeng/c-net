
/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include <opencv2/core/types.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/mat.hpp>

extern "C" {
#include "detection/retina_face.h"
#include "face/face_info.h"
#include "lpr/plate_info.h"
#include "tensor_pixel.h"
#include "models/plate/plate-opt.h"
}

struct Object {
    cv::Rect_<float> rect;
    std::vector<cv::Point2i> landmarks;
    int label;
    float prob;
};

static void draw_objects(const cv::Mat &bgr, const std::vector<Object> &objects) {
    cv::Mat image = bgr.clone();

    for (const auto &obj : objects) {
        fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
                obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);

        int x = obj.rect.x;
        int y = obj.rect.y;
        if (y < 0)
            y = 0;
        if (x > image.cols)
            x = image.cols;

        cv::rectangle(image, cv::Rect(cv::Point(x, y), cv::Size(obj.rect.size())), cv::Scalar(0, 0, 255), 1);
        for (const auto &landmark : obj.landmarks) {
            circle(image, landmark, 1, cv::Scalar(0, 255, 0), 1);
        }
    }

    cv::imwrite("image.png", image);
    cv::waitKey(0);
}


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s  [image_path]\n", argv[0]);
        return -1;
    }

    const char *image_path = argv[1];
    cv::Mat bgr = cv::imread(image_path, 1);
    if (bgr.empty()) {
        fprintf(stderr, "cv::imread %s failed\n", image_path);
        return -1;
    }

    tensor_t input = tensor_from_pixels_resize(bgr.data, PIXEL_BGR, bgr.cols, bgr.rows, 320, 240, nullptr);
    const float mean_values[3] = {104.f, 117.f, 123.f};
    tensor_substract_mean_normalize(&input, mean_values, NULL);

    /**
     * set up run operations
     */
    option_init(8);
    option_t opt = option_get_instance();
    network_setup_run_operations(&network_plate_opt, &opt);

    /**
     * run network
     */
    session_t session = session_create(&network_plate_opt);
    opt.light_mode = 1;
    session.option = opt;
    session_set_input(&session, plate_opt_blob_input0_input0, input);
    tensor_release(&input);
    session_run(&session);

    /**
     * get tensors
     */
    vector_tensor_t outputs = vector_tensor_create_default(3);
    outputs.data[0] = session_get_output(&session, plate_opt_blob_output0_output0);
    outputs.data[1] = session_get_output(&session, plate_opt_blob_445_530);
    outputs.data[2] = session_get_output(&session, plate_opt_blob_444_529);
    session_release(&session);
    network_teardown_run_operations(&network_plate_opt, &opt);

    retina_face_config config;
    config.width = 320;
    config.height = 240;
    config.nms = 0.4;
    config.threshold = 0.02;
    config.retain_face = 0;
    config.land_type = 0;

    /**
     * forward retain_face
     */
    retina_face_t face = retina_face_create(config);
    linked_list detections = create_linked_list(nullptr);
    retina_face_forward(&face, &outputs, &detections, nullptr);
    retina_face_release(&face);
    vector_tensor_free(&outputs);
    option_uninit();

    int img_w = bgr.cols;
    int img_h = bgr.rows;
    linked_node *iter = detections.head;
    std::vector<Object> objects;
    while (iter != nullptr) {
        plate_info_t &info = ((plate_info_node *) iter)->plate_info;
        Object object;
        object.label = info.bbox.classid + 1;
        object.prob = info.bbox.score;
        object.rect.x = info.bbox.x_min * img_w;
        object.rect.y = info.bbox.y_min * img_h;
        object.rect.width = info.bbox.x_max * img_w - object.rect.x;
        object.rect.height = info.bbox.y_max * img_h - object.rect.y;
        object.landmarks.resize(4);
        object.landmarks[0].x = info.landmark[0] * img_w;
        object.landmarks[0].y = info.landmark[1] * img_h;
        object.landmarks[1].x = info.landmark[2] * img_w;
        object.landmarks[1].y = info.landmark[3] * img_h;
        object.landmarks[2].x = info.landmark[4] * img_w;
        object.landmarks[2].y = info.landmark[5] * img_h;
        object.landmarks[3].x = info.landmark[6] * img_w;
        object.landmarks[3].y = info.landmark[7] * img_h;

        objects.push_back(object);
        iter = iter->next;
    }

    destroy_linked_list(&detections);
    draw_objects(bgr, objects);

    return 0;
}