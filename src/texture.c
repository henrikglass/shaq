/*--- Include files ---------------------------------------------------------------------*/

#include "texture.h"
#include "alloc.h"
#include "log.h"
#include "glad/glad.h"

static inline void *stbi_alloc(size_t size);
static inline void *stbi_realloc(void *ptr, size_t size);
static inline void stbi_free(void *ptr);
static inline void *stbi_alloc(size_t size){ return hgl_alloc(g_session_fs_allocator, size);}
static inline void *stbi_realloc(void *ptr, size_t size){ return hgl_realloc(g_session_fs_allocator, ptr, size);}
static inline void stbi_free(void *ptr){ /*(void) ptr;*/ hgl_free(g_session_fs_allocator, ptr); }
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

#include "hgl_profile.h"

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

Texture texture_load_from_file(StringView filepath)
{
    hgl_profile_begin("texture_load_from_file#load");
    const char *filepath_cstr = sv_make_cstr_copy(filepath, tmp_alloc);
    Texture tex = {
        .filepath = filepath,
    };
    stbi_set_flip_vertically_on_load(1);
    tex.data = stbi_load(filepath_cstr, &tex.w, &tex.h, &tex.n_channels, 0);

    if (tex.data == NULL) {
        hgl_profile_end();
        return tex;
    }
    hgl_profile_end();

    hgl_profile_begin("texture_load_from_file#gl");
    glGenTextures(1, &tex.gl_texture_id); 
    glBindTexture(GL_TEXTURE_2D, tex.gl_texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    switch (tex.n_channels) {
        case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.w, tex.h, 0, GL_RED, GL_UNSIGNED_BYTE, tex.data); break;
        case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.w, tex.h, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.data); break;
        case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.w, tex.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.data); break;
        default: log_error("Texture loaded from `" SV_FMT "` has %d channels, for some reason?",
                           SV_ARG(filepath), tex.n_channels);
    }
    hgl_profile_end();

    return tex;
}

Texture texture_make_empty(IVec2 resolution)
{
    Texture tex = {0};

    glGenTextures(1, &tex.gl_texture_id); 
    glBindTexture(GL_TEXTURE_2D, tex.gl_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution.x, resolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return tex;
}

void texture_free(Texture *t)
{
    glDeleteTextures(1, &t->gl_texture_id); 
}


/*--- Private functions -----------------------------------------------------------------*/

