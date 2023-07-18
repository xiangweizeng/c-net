/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/



#include "stdio.h"
#include "operation_config.h"

float operation_spend_times[prelu_operation_type + 1];
float operation_spend_time(operation_type_t type, double spend_time)
{
    operation_spend_times[type] += spend_time;
    return operation_spend_times[type];
}

void clear_spend_time()
{
    for(int i = 0; i <= prelu_operation_type; i++){
        operation_spend_times[i] = 0;
    }
}

void print_spend_times(){
    float all_total = 0;
    for(int i = 0; i <= prelu_operation_type; i++){
        all_total += operation_spend_times[i];
    }

    printf("benchmark all operation total: %f\n", all_total);
    for(int i = 0; i <= prelu_operation_type; i++){
        if(operation_spend_times[i] == 0.f)continue;
        printf("op: %s total: %f(%f)\n",
               operation_names[i],
               operation_spend_times[i],
               operation_spend_times[i] / all_total);
    }
}