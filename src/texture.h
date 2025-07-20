#ifndef TEXTURE_H
#define TEXTURE_H

/*--- Include files ---------------------------------------------------------------------*/

#include "str.h"

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
    // TODO GL FORMAT
    u8 *data;
} Texture;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

Texture texture_load_from_file(StringView filepath);

#endif /* TEXTURE__H */

