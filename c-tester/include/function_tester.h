/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_FUNCTION_TESTER_H
#define CNET_FUNCTION_TESTER_H

#include "tester.h"

/**
 * tester declar for function tester
 */
#define TESTER_DECLAR(impl, create_args_t)\
void run_##impl##_example();\
typedef void (*impl##_test_function)(create_args_t *);\
void tester_##impl_set_setup(impl##_test_function);\
void tester_##impl_set_tear_down(impl##_test_function);\
typedef struct example_##impl##_test_t{\
    char name[256];\
    create_args_t args;\
}example_##impl##_test_t;

/**
 * run function test examples
 */
#define TESTER_EXAMPLES_RUN(impl) run_##impl##_example()

/**
 * set function testeer setup
 */
#define TESTER_SET_SETUP(impl, fun) tester_##impl##_set_setup(fun)

/**
 * set function tester tear down
 */
#define TESTER_SET_TEAR_DOWN(impl, fun) tester_##impl##_set_tear_down(fun)

/**
 * impl examples, name and args
 */
#define TESTER_EXAMPLES(impl) \
example_##impl##_test_t impl##_examples[] =

/**
 * function tester impl
example:
int add(int a, int b)
{
    return a+b;
}

typedef struct add_create_args_t{
    int a;
    int b;
    int expect;
}add_create_args_t;

void test_add(add_create_args_t *args){
    TEST_ASSERT_EQUAL_INT(args->expect, add(args->a, args->b));
}

TESTER_DECLAR(add, add_create_args_t)
TESTER_EXAMPLES(add){
        {.name = "add", .args = { 1 ,  1,  2}},
        {.name = "add", .args = {-1 , -2, -2}},
};

TESTER_IMPL(add, add_create_args_t, test_add)

int main()
{
    TESTER_EXAMPLES_RUN(add);
}
 */
#define TESTER_IMPL(impl, create_args_t, test_function) \
static test_status_t impl##_create_context(tester_t* tester)\
{\
    tester->context = test_function;\
    return  TEST_SUCCESS;\
}\
\
static impl##_test_function impl##_test_setup_fun = NULL;\
static impl##_test_function impl##_test_tear_down_fun = NULL;\
static test_status_t impl##_test_run(tester_t* tester)\
{\
    if(NULL == tester->context){\
        return TEST_INVALID;\
    }\
\
    create_args_t *args = (create_args_t*)tester->create_args;\
    if(TEST_PROTECT()){\
        if(NULL != impl##_test_setup_fun){\
            impl##_test_setup_fun(args);\
        }\
        ((impl##_test_function)(tester->context))(args);\
    }\
\
    if(TEST_PROTECT()){\
        if(NULL != impl##_test_tear_down_fun){\
            impl##_test_tear_down_fun(args);\
        }\
    }\
\
    if(TEST_SUCCESS != Unity.CurrentTestFailed)\
    {\
        return TEST_RUN_FAILED;\
    }\
\
    return  TEST_SUCCESS;\
}\
\
tester_t tester_create_##impl(example_##impl##_test_t *example)\
{\
    tester_t tester= {\
            .run_status = TEST_INVALID,\
            .create_args = &example->args,\
            .context = NULL,\
            .create_context =  impl##_create_context,\
            .run_reference = NULL,\
            .run = impl##_test_run,\
            .run_verify = NULL,\
            .release_context = NULL\
    };\
\
    strcpy(tester.tester_name, example->name);\
    return tester;\
}\
\
void tester_##impl##_set_setup(impl##_test_function fun)\
{\
    impl##_test_setup_fun = fun;\
}\
\
void tester_##impl##_set_tear_down(impl##_test_function fun)\
{\
    impl##_test_tear_down_fun = fun; \
}\
\
void run_##impl##_example()\
{\
    CNET_TEST_BEGIN();\
    int size = sizeof(impl##_examples) / (sizeof(struct example_##impl##_test_t));\
    for(int i= 0 ; i < size; i ++){\
        tester_t tester = tester_create_##impl(&(impl##_examples[i]));\
        tester_run(&tester);\
    }\
\
    CNET_TEST_END();\
}

#endif //CNET_FUNCTION_TESTER_H
