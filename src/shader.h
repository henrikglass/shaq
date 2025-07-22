#ifndef SHADER_H
#define SHADER_H

/*--- Include files ---------------------------------------------------------------------*/

#include "constants.h"
#include "uniform.h"
#include "texture.h"
#include "array.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct {
    //const char *name;
    //const char *src;
    StringView name;
    StringView filepath;
    u8 *frag_shader_src;
    size_t frag_shader_src_size;
    i64 modifytime;
    Array(Uniform, SHAQ_MAX_N_SHADERS) uniforms;
    Array(u32, SHAQ_MAX_N_SHADERS) shader_depends;
    Texture render_texture;

    /* OpenGL */
    u32 gl_shader_program_id;
} Shader;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

i32 shader_parse_from_ini_section(Shader *sh, HglIniSection *s);
void shader_determine_dependencies(Shader *s);
b8 shader_needs_reload(Shader *s);
void shader_reload(Shader *s);
void shader_make_last_pass_shader(Shader *s);
void shader_free_opengl_resources(Shader *s);
void shader_prepare_for_drawing(Shader *s);

#endif /* SHADER_H */

