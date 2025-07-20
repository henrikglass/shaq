/*--- Include files ---------------------------------------------------------------------*/

#include "texture.h"

#include "alloc.h"

void *stbi_alloc(size_t size);
void *stbi_realloc(void *ptr, size_t size);
void stbi_free(void *ptr);
void *stbi_alloc(size_t size){ return fs_alloc(g_longterm_fs_allocator, size);}
void *stbi_realloc(void *ptr, size_t size){ return fs_realloc(g_longterm_fs_allocator, ptr, size);}
void stbi_free(void *ptr){ /*(void) ptr;*/ fs_free(g_longterm_fs_allocator, ptr); }
#define STBI_MALLOC stbi_alloc
#define STBI_REALLOC stbi_realloc
#define STBI_FREE stbi_free
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma GCC diagnostic pop

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

Texture texture_load_from_file(StringView filepath)
{
    const char *filepath_cstr = sv_make_cstr_copy(filepath);
    Texture tex = {
        .filepath = filepath,
        .n_channels = 4,
    };
    stbi_set_flip_vertically_on_load(1);
    tex.data = stbi_load(filepath_cstr, &tex.w, &tex.h, &tex.n_channels, tex.n_channels);
    return tex;
}

/*--- Private functions -----------------------------------------------------------------*/

