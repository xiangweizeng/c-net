/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

#include <string.h>
#include <allocator.h>
#include "imgproc.h"

#ifdef STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

void image_create(struct image_t *img, uint16_t w, uint16_t h, uint16_t c, image_type_t type) {
    // Set the variables
    img->type = type;
    img->w = w;
    img->h = h;
    img->c = c;

    // Depending on the type the size differs
    if (PIXEL_YUV_DATA == type) {
        img->buf_size = sizeof(uint8_t) * 2 * img->w * img->h;
    } else if (PIXEL_WHCF_DATA == type) {
        img->buf_size = sizeof(float) * c * img->w * img->h;
    } else {
        img->buf_size = sizeof(uint8_t) * c * img->w * img->h;
    }

    img->buf = fast_malloc(img->buf_size);
}

void image_create_data(image_t *img, uint16_t w, uint16_t h, uint16_t c, image_type_t type, void *data) {
    // Set the variables
    img->type = type;
    img->w = w;
    img->h = h;
    img->c = c;

    // Depending on the type the size differs
    if (PIXEL_YUV_DATA == type) {
        img->buf_size = sizeof(uint8_t) * 2 * img->w * img->h;
    } else if (PIXEL_WHCF_DATA == type) {
        img->buf_size = sizeof(float) * c * img->w * img->h;
    } else {
        img->buf_size = sizeof(uint8_t) * c * img->w * img->h;
    }

    img->buf = data;
}

void image_free(struct image_t *img) {
    if (NULL != img->buf) {
        fast_free(img->buf);
        img->buf = NULL;
    }
}

void image_copy(struct image_t *input, struct image_t *output) {
    if (input->type != output->type) {
        return;
    }

    output->w = input->w;
    output->h = input->h;
    output->c = input->c;
    output->buf_size = input->buf_size;
    memcpy(output->buf, input->buf, input->buf_size);
}


#ifdef STB_IMAGE
image_t load_image(char* file_name)
{
    int w, h, c;
    unsigned char *data = stbi_load(file_name, &w, &h, &c, 3);

    image_type_t image_type = 0;
    if(c == 1){
        image_type = PIXEL_GARY_DATA;
    }else if(c == 3){
        image_type = PIXEL_BGR_DATA;
    }

    image_t image;
    memset(&image, 0, sizeof(image));

    if(NULL != data){
        image_create_data(&image, w , h, c, image_type, data);
    }

    return image;
}

int save_image(image_t image, image_file_type_t type, char *file_name)
{
    char *format = "%s.%s";
    int ret = 0;
    char *buffer_name = (char*)fast_malloc(strlen(file_name) + 6);
    switch(type){
        case IMAGE_FILE_TYPE_BMP:{
            sprintf(buffer_name, format, file_name, "bmp");
            ret = stbi_write_bmp(buffer_name, image.w, image.h, image.c, image.buf);
            break;
        }
        case IMAGE_FILE_TYPE_JPEG:{
            sprintf(buffer_name, format, file_name, "jpeg");
            ret = stbi_write_jpg(buffer_name, image.w, image.h, image.c, image.buf, 100);
            break;
        }
        case IMAGE_FILE_TYPE_PNG:{
            sprintf(buffer_name, format, file_name, "png");
            ret = stbi_write_png(buffer_name, image.w, image.h, image.c, image.buf, image.w * image.c);
            break;
        }
    }

    fast_free(buffer_name);
    return ret;
}

#endif
