#ifndef SHADER_H
#define SHADER_H

/*--- Include files ---------------------------------------------------------------------*/

#include "shaq_config.h"
#include "uniform.h"
#include "texture.h"
#include "array.h"
#include "str.h"
#include "vecmath.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct {
    StringView name;

    struct {
        StringView source;
        IVec2 resolution;
        i32 format;
        Array(StringView, SHAQ_MAX_N_SHADERS) render_after;
    } attributes;

    Array(Uniform, SHAQ_MAX_N_SHADERS) uniforms;
    Array(u32, SHAQ_MAX_N_SHADERS) shader_depends;
    Texture render_texture[2];
    Texture *render_texture_current;
    Texture *render_texture_last;

    u8 *frag_shader_src;
    size_t frag_shader_src_size;
    i64 modifytime;

    /* OpenGL */
    u32 gl_shader_program_id;
} Shader;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

i32 shader_parse_from_ini_section(Shader *sh, HglIniSection *s);
void shader_determine_dependencies(Shader *s);
b8 shader_was_modified(Shader *s);
b8 shader_is_ok(const Shader *s);
void shader_reload(Shader *s);
void shader_make_last_pass_shader(Shader *s);
void shader_free_opengl_resources(Shader *s);
void shader_swap_render_textures(Shader *s);
void shader_update_uniforms(Shader *s);
Uniform *shader_find_uniform_by_name(Shader *s, StringView name);

#endif /* SHADER_H */

