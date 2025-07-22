/*--- Include files ---------------------------------------------------------------------*/

#include "texture.h"
#include "alloc.h"
#include "glad/glad.h"

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

    glGenTextures(1, &tex.gl_texture_id); 
    glBindTexture(GL_TEXTURE_2D, tex.gl_texture_id);

    /* TODO Add support for specifying these in the *.ini file? */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    switch (tex.n_channels) {
        case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex.w, tex.h, 0, GL_RED, GL_UNSIGNED_BYTE, tex.data); break;
        case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.w, tex.h, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.data); break;
        case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.w, tex.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.data); break;
        default: printf("uhoh channels?\n");
    }

    return tex;
}

Texture texture_make_empty()
{
    Texture tex = {0};

    glGenTextures(1, &tex.gl_texture_id); 
    glBindTexture(GL_TEXTURE_2D, tex.gl_texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return tex;
}

void texture_free_opengl_resources(Texture *t)
{
    glDeleteTextures(1, &t->gl_texture_id); 
}


/*--- Private functions -----------------------------------------------------------------*/

