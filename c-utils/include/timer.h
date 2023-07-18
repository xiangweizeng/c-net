/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_TIMER_H
#define CNET_TIMER_H

#include <time.h>

static inline double what_time_is_it_now()
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return 1000 *(now.tv_sec + now.tv_nsec*1e-9);
}

static inline void print_spend_time(char *message, double start)
{
    printf("\n'time_test %s:  %f ms.\n", message, what_time_is_it_now() - start);
}

#define TIME_MARK_BEGIN(function_name)   double start_##function_name = what_time_is_it_now();
#define TIME_MARK_END(function_name)     (what_time_is_it_now() -  start_##function_name)
#define TIME_MARK_PRINT(function_name)   print_spend_time(#function_name, start_##function_name);

#endif //CNET_TIMER_H
