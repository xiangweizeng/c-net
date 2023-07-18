/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#ifndef CNET_OPERATION_BENCHMARK_H
#define CNET_OPERATION_BENCHMARK_H

#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

float operation_spend_time(operation_type_t type, double spend_time);
void clear_spend_time();
void print_spend_times();

#endif //CNET_OPERATION_BENCHMARK_H
