/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef NCNN_UTILS_H
#define NCNN_UTILS_H

#include <vector>
#include <getopt.h>
#include <cstring>
#include <vector>
#include <dirent.h>
#include <iostream>


/**
 * Get the filenames from direct path
 * @param base_path
 * @param file_path
 * @return
 */
static int parse_images_dir(const char *base_path, std::vector<std::string> &file_path) {
    DIR *dir;
    struct dirent *ptr;

    if ((dir = opendir(base_path)) == nullptr) {
        perror("Open dir error...");
        exit(1);
    }

    while ((ptr = readdir(dir)) != nullptr) {
        /// current dir OR parent dir
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }

        std::string path = base_path;
        file_path.push_back(path + ptr->d_name);
    }
    closedir(dir);

    return 0;
}

#define FILE_TYPE 0
#define DIRECTORY 1

static std::vector<std::string> get_all_files(const char *dir, const std::string& filetype, int fileOrDir) {
    std::vector<std::string> dir_lists, file_lists;
    dir_lists.clear();
    file_lists.clear();

    std::string str_cur_dir;
    std::string str_sub_dir;
    std::string str_file;

    str_cur_dir = dir;
    if (str_cur_dir.length() == 0) {
        return file_lists;
    }

    if (str_cur_dir.at(str_cur_dir.length() - 1) != '/') {
        str_cur_dir += "/";
    }

    dir_lists.push_back(str_cur_dir);
    DIR *dp;
    struct dirent *dirent_p;

    size_t k = filetype.size(), n;
    std::string str_temp;

    while (!dir_lists.empty()) {
        str_cur_dir = dir_lists.back();
        dir_lists.pop_back();

        if ((dp = opendir(str_cur_dir.c_str())) == nullptr) {
            std::cout << "ERROR:Open " << str_cur_dir << " failed!" << std::endl;
            continue;
        }
        while ((dirent_p = readdir(dp)) != nullptr) {
            str_file.clear();
            str_file = str_cur_dir;
            str_file += dirent_p->d_name;

            if (strcmp(dirent_p->d_name, ".") == 0 ||
                strcmp(dirent_p->d_name, "..") == 0) {
                continue;
            }

            if (fileOrDir == FILE_TYPE) {
                if (dirent_p->d_type == DT_DIR) {
                    str_sub_dir.clear();
                    str_sub_dir = str_cur_dir + dirent_p->d_name;
                    str_sub_dir += "/";
                    dir_lists.push_back(str_sub_dir);
                    continue;
                }

                if (!filetype.empty()) {
                    n = str_file.size();
                    if (str_file.at(n - k - 1) != '.')
                        continue;
                    str_temp.clear();
                    str_temp = str_file.substr(n - k, k);
                    if (!(filetype == str_temp))
                        continue;
                }

                file_lists.push_back(str_file);
            } else {
                if (dirent_p->d_type != DT_DIR) {
                    continue;
                } else {
                    str_sub_dir.clear();
                    str_sub_dir = str_cur_dir + dirent_p->d_name;
                    str_sub_dir += "/";
                    file_lists.push_back(str_sub_dir);
                    dir_lists.push_back(str_sub_dir);
                }
            }
        }
        closedir(dp);
    }
    return file_lists;
}

#endif //NCNN_UTILS_H
