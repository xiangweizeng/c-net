/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "tester.h"

static void tester_release_context(tester_t *tester) {
    if(NULL != tester->release_context && TEST_SUCCESS != tester->release_context(tester)){
        tester->run_status = TEST_RELEASE_CONTEXT_FAILED;
    }
}

/**
 * run impl
 * @param tester
 * @return
 */
test_status_t tester_run(tester_t *tester)
{
    if(NULL == tester || NULL == tester->run){
        return TEST_INVALID;
    }

    Unity.CurrentTestName = tester->tester_name;
    Unity.NumberOfTests++;
    UNITY_CLR_DETAILS();

    /**
     * create context
     */
    tester->run_status = TEST_SUCCESS;
    if(NULL != tester->create_context && TEST_SUCCESS != tester->create_context(tester)){
        tester->run_status = TEST_CREATE_CONTEXT_FAILED;
        goto TEST_END;
    }

    /**
     * run
     */
     if(TEST_PROTECT()){
         if(TEST_SUCCESS != tester->run(tester)){
             tester_release_context(tester);
             tester->run_status = TEST_RUN_FAILED;
             goto TEST_END;
         }
     }

    /**
    * reference run
    */
    if(TEST_PROTECT()){
        if(NULL != tester->run_reference && TEST_SUCCESS != tester->run_reference(tester)){
            tester_release_context(tester);
            tester->run_status = TEST_REFERENCE_RUN_FAILED;
            goto TEST_END;
        }
    }

    /**
     * ｖerify run
     */
     if(TEST_PROTECT()){
         if(NULL != tester->run_verify && TEST_SUCCESS != tester->run_verify(tester)){
             tester_release_context(tester);
             tester->run_status = TEST_VERIFY_RUN_FAILED;
             goto TEST_END;
         }
     }

    /**
     * release context
     */
    if(NULL != tester->release_context && TEST_SUCCESS != tester->release_context(tester)){
        tester->run_status = TEST_RELEASE_CONTEXT_FAILED;
        goto TEST_END;
    }


    TEST_END:
    UnityConcludeTest();
    return tester->run_status;
}