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
    u8 *source_code;
    size_t source_code_size;
    i64 modifytime;
    Array(Uniform, SHAQ_MAX_N_SHADERS) uniforms;
    Array(u32, SHAQ_MAX_N_SHADERS) shader_depends;
    Texture outimage;
} Shader;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

i32 shader_parse_from_ini_section(Shader *sh, HglIniSection *s);
void shader_determine_dependencies(Shader *s);
b8 shader_needs_reload(Shader *s);

#endif /* SHADER_H */

