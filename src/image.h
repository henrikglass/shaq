#ifndef IMAGE_H
#define IMAGE_H

/*--- Include files ---------------------------------------------------------------------*/

#include "str.h"
#include "hgl_int.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    StringView filepath;
    void *data;
    i32 width;
    i32 height;
    i32 n_channels;
} Image;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

Image *image_load_from_file(StringView filepath);
void image_free_all_cached_images(void);

#endif /* IMAGE_H */

