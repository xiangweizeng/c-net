/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_TESTER_BASE_H
#define CNET_TESTER_BASE_H

#include <string.h>
#include "unity/unity.h"

static const char* get_base_name(const char* fname) {
    int len = strlen(fname);
    const char* s = fname + len;
    while(s > fname) {
        if(s[-1] == '/' || s[-1] == '\\') return s;
        s--;
    }
    return s;
}

#define CNET_TEST_BEGIN()  UnityBegin(get_base_name(__FILE__));   Unity.CurrentTestLineNumber = __LINE__
#define CNET_TEST_END()    UnityEnd()

#endif //CNET_TESTER_BASE_H
