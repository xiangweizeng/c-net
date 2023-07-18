/**
 *
 * Copyright (c) 2023 C-NET
 * All rights reserved.
 * Created by XiangWei-Zeng
 *
 **/

/*
 * File:   imgproc.h
 * Author: damone
 *
 * Created on 2017年11月11日, 上午11:21
 */

#ifndef IMGPROC_H
#define IMGPROC_H

#include <inttypes.h>
#include <limits.h>

#define bit_is_set(x, b) ((x >> b) & 0x1)
#define _BV(bit) (1 << (bit))
#define set_bit(a, n) a |= (1 << n)
#define clear_bit(a, n) a &= ~(1 << n)
#define rad_of_deg(x) ((x) * (M_PI / 180.))

#define Min(x, y) ((x) < (y) ? (x) : (y))
#define Max(x, y) ((x) > (y) ? (x) : (y))
#define ABS(val) ((val) < 0 ? -(val) : (val))

/* The different type of images we currently support */
typedef enum image_type_t {
    /**
     * image type
     * RGB
     * BGR
     * YUV
     * WHCF
     */
    PIXEL_RGB_DATA = 1,
    PIXEL_BGR_DATA = (1 << 1),
    PIXEL_YUV_DATA = (1 << 2),
    PIXEL_WHCF_DATA = (1 << 3),
    PIXEL_GARY_DATA = (1 << 4),
}image_type_t;

/**
 * Main image structure
 */
typedef struct image_t {
    image_type_t type;         ///< The image type
    uint16_t w;                ///< Image width
    uint16_t h;                ///< Image height
    uint16_t c;                ///< Image channls

    uint32_t buf_size;         ///< The buffer size
    void *buf;                 ///< Image buffer (depending on the image_type)
}image_t;

/**
 * Image point structure
 */
typedef struct cnet_point_t {
    int32_t x;
    int32_t y;
}cnet_point_t;

/**
 * @brief create image, include buffer
 * 
 * @param img 
 * @param w 
 * @param h 
 * @param c 
 * @param type 
 */
void image_create(image_t *img, uint16_t w, uint16_t h, uint16_t c, image_type_t type);

/**
 * create image , give a data pointer
 * @param img
 * @param w
 * @param h
 * @param c
 * @param type
 * @param data
 */
void image_create_data(image_t *img, uint16_t w, uint16_t h, uint16_t c, image_type_t type, void *data);

/**
 * @brief copy image, image type is small, images must be created
 * 
 * @param input 
 * @param output 
 */
void image_copy(struct image_t *input, struct image_t *output);

/**
 * @brief relsease image
 * 
 * @param img 
 */
void image_free(struct image_t *img);


/**
 * load image from file
 * @param file_name
 * @return
 */
image_t load_image(char* file_name);

/**
 * image file type
 */
typedef enum image_file_type_t{
    IMAGE_FILE_TYPE_PNG = 1,
    IMAGE_FILE_TYPE_JPEG = 2,
    IMAGE_FILE_TYPE_BMP = 3
}image_file_type_t;

/**
 * save image to png file
 * @note auto add file suffix
 * @param image
 * @param file_name
 * @return
 */
int save_image(image_t image, image_file_type_t, char *file_name);

#endif /* IMGPROC_H */
