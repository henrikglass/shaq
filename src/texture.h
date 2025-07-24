#ifndef TEXTURE_H
#define TEXTURE_H

/*--- Include files ---------------------------------------------------------------------*/

#include "str.h"
#include "vecmath.h"
#include "hgl_int.h"
#include "hgl_float.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    StringView filepath;
    i32 w;
    i32 h;
    i32 n_channels;
    u8 *data;

    /* OpenGL */
    u32 gl_texture_id;
} Texture;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

Texture texture_load_from_file(StringView filepath);
Texture texture_make_empty(IVec2 resolution);
void texture_free_opengl_resources(Texture *t);

#endif /* TEXTURE__H */

