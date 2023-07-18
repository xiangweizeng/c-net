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
#include <string.h>
#include <vector>
#include <dirent.h>
#include <iostream>


// Get the filenames from direct path
static int parse_images_dir(const char *base_path, std::vector<std::string>& file_path)
{
    DIR *dir;
    struct dirent *ptr;

    if ((dir=opendir(base_path)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
        {
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
static std::vector<std::string> getall(const char *dir, std::string filetype, int fileOrDir) {
    std::vector<std::string> dirlists, filelists;
    dirlists.clear();
    filelists.clear();

    std::string strCurDir = "";
    std::string strSubDir = "";
    std::string strFile = "";

    strCurDir = dir;
    if (strCurDir.length() == 0) {
        return filelists;
    }

    if (strCurDir.at(strCurDir.length() - 1) != '/') {
        strCurDir += "/";
    }

    dirlists.push_back(strCurDir);
    DIR *dp;
    struct dirent *direntp;

    int k = filetype.size();
    int n = 0;
    std::string strTemp = "";

    while (!dirlists.empty()) {
        strCurDir = dirlists.back();
        dirlists.pop_back();
        dp = NULL;

        if ((dp = opendir(strCurDir.c_str())) == NULL) {
            std::cout << "ERROR:Open " << strCurDir << " failed!" << std::endl;
            continue;
        }
        while ((direntp = readdir(dp)) != NULL) {
            strFile.clear();
            strFile = strCurDir;
            strFile += direntp->d_name;

            if (strcmp(direntp->d_name, ".") == 0 ||
                strcmp(direntp->d_name, "..") == 0) {
                continue;
            }

            if (fileOrDir == FILE_TYPE)
            {
                if (direntp->d_type == DT_DIR)
                {
                    strSubDir.clear();
                    strSubDir = strCurDir + direntp->d_name;
                    strSubDir += "/";
                    dirlists.push_back(strSubDir);
                    continue;
                }

                if (filetype.size() != 0) {
                    n = strFile.size();
                    if (strFile.at(n - k - 1) != '.')
                        continue;
                    strTemp.clear();
                    strTemp = strFile.substr(n - k, k);
                    if (!(filetype == strTemp))
                        continue;
                }

                filelists.push_back(strFile);
            } else{
                if (direntp->d_type != DT_DIR)
                {
                    continue;
                } else {
                    strSubDir.clear();
                    strSubDir = strCurDir + direntp->d_name;
                    strSubDir += "/";
                    filelists.push_back(strSubDir);
                    dirlists.push_back(strSubDir);
                }
            }
        }
        closedir(dp);
    }
    return filelists;
}

#endif //NCNN_UTILS_H
