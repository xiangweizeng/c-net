/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_TESTER_H
#define CNET_TESTER_H

#include <stdlib.h>
#include "tester_base.h"
/**
 * impl status
 */
typedef enum test_status_t{
    /**
     * invalid
     */
    TEST_INVALID = -1,

    /**
     * benTEST run success
     */
    TEST_SUCCESS = 0,

    /**
     * create context failed
     */
    TEST_CREATE_CONTEXT_FAILED = 1,

    /**
     * reference run failed
     */
    TEST_REFERENCE_RUN_FAILED = 2,

    /**
     * run failed
     */
    TEST_RUN_FAILED = 3,

    /**
     * verify run failed
     */
    TEST_VERIFY_RUN_FAILED = 4,

    /**
     * release context failed
     */
    TEST_RELEASE_CONTEXT_FAILED = 5,
}test_status_t;

static const char* test_status_str(test_status_t status){
    switch (status){
        case TEST_INVALID:
            return "INVALID";
        case TEST_SUCCESS:
            return "SUCCESS";
        case TEST_CREATE_CONTEXT_FAILED:
            return "CONTEXT FAILED";
        case TEST_REFERENCE_RUN_FAILED:
            return "REFER   FAILED";
        case TEST_RUN_FAILED:
            return "RUN     FAILED";
        case TEST_VERIFY_RUN_FAILED:
            return "VERIFY  FAILED";
        case TEST_RELEASE_CONTEXT_FAILED:
            return "RELEASE FAILED";
        default:
            return "";
    }
}

/**
 * benchmakr_t
 *
 * basic data
 * contxt
 * function
 */
struct tester_t;
typedef struct tester_t tester_t;

/**
 * benchmakr function
 */
typedef test_status_t (*test_run_fun)(tester_t* tester);

struct tester_t{
    /**
     * basic info
     */
    char tester_name[256];
    void *create_args;
    test_status_t run_status;

    /**
     * context, create by create_cotext, release by release release_context
     */
    void *context;

    /**
     * get impl name and tag from create args and impl
     */
    test_run_fun create_tester_name;

    /**
     * create context function
     */
    test_run_fun create_context;

    /**
     * run function
     */
    test_run_fun run;

    /**
     * reference run function
     */
    test_run_fun run_reference;

    /**
    * run verify
    */
    test_run_fun run_verify;

    /**
     * release contxt function
     */
    test_run_fun release_context;
};

/**
 * create default tester
 * @return
 */
static inline tester_t tester_create_default()
{
    tester_t tester = {
            .tester_name ={0},
            .run_status = TEST_INVALID,
            .create_args = NULL,
            .context = NULL,
            .create_context = NULL,
            .run_reference = NULL,
            .run = NULL,
            .run_verify = NULL,
            .release_context = NULL
    };

    return tester;
}

/**
 * run impl
 * @param tester
 * @return
 */
test_status_t tester_run(tester_t *tester);

#endif //CNET_TESTER_H
