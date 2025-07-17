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
    HglStringView name;
    HglStringView src;
    Array(Uniform, SHAQ_MAX_N_SHADERS) uniforms;
    Array(u32, SHAQ_MAX_N_SHADERS) shader_depends;
    Texture outimage;
} Shader;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void shader_parse_from_ini_section(Shader *sh, HglIniSection *s);
void shader_determine_dependencies(Shader *s);

#endif /* SHADER_H */

