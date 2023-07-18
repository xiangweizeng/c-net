/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include "function_tester.h"

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
        {.name = "add_error", .args = {-1 , -2, -2}},
};

TESTER_IMPL(add, add_create_args_t, test_add)

int main()
{
    TESTER_EXAMPLES_RUN(add);
}