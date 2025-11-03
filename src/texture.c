/*--- Include files ---------------------------------------------------------------------*/

#include "texture.h"
#include "alloc.h"
#include "log.h"
#include "glad/glad.h"

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

Texture texture_load_from_file(StringView filepath)
{
    Texture tex = {0};
    tex.img = image_load_from_file(filepath); 
    if (tex.img->data == NULL) {
        return tex;
    }

    glGenTextures(1, &tex.gl_texture_id); 
    glBindTexture(GL_TEXTURE_2D, tex.gl_texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    switch (tex.img->n_channels) {
        case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.img->width, tex.img->height, 0, GL_RED, GL_UNSIGNED_BYTE, tex.img->data); break;
        case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.img->width, tex.img->height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.img->data); break;
        case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.img->width, tex.img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.img->data); break;
        default: log_error("Texture loaded from `" SV_FMT "` has %d channels, for some reason?",
                           SV_ARG(filepath), tex.img->n_channels);
    }

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

