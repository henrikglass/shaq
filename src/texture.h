#ifndef TEXTURE_H
#define TEXTURE_H

/*--- Include files ---------------------------------------------------------------------*/

#include "str.h"
#include "vecmath.h"
#include "hgl_int.h"
#include "hgl_float.h"
#include "image.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    Image *img;
    u32 gl_texture_id;
} Texture;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

Texture texture_load_from_file(StringView filepath);
Texture texture_make_empty(IVec2 resolution, i32 internal_format);
void texture_free(Texture *t);

#endif /* TEXTURE__H */

