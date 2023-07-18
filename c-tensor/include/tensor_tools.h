/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#ifndef CNET_TENSOR_TOOLS_H
#define CNET_TENSOR_TOOLS_H

#include <stdio.h>
#include <tensor.h>

void tensor_to_carray(tensor_t *tensor, char *file_name,  char * name)
{
    FILE* mp = fopen(file_name, "wb");

    if (!mp){
        fprintf(stderr, "fopen %s failed\n", file_name);
        return;
    }

    fprintf(mp, "\n#ifdef _MSC_VER\n__declspec(align(4))\n#else\n__attribute__((aligned(4)))\n#endif\n");
    fprintf(mp, "unsigned char %s[] = {\n", name);

    int i = 0;
    int total = tensor_total(tensor) * tensor->elem_size;
    unsigned char *data = (unsigned char*)tensor->data;
    while(i < total){

        fprintf(mp, "0x%02x,", data[i]);

        i++;
        if (i % 16 == 0)
        {
            fprintf(mp, "\n");
        }
    }

    fprintf(mp, "};\n");
    fclose(mp);
}


#endif //CNET_TENSOR_TOOLS_H
