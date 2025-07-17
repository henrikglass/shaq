/*--- Include files ---------------------------------------------------------------------*/

#include "shader.h"

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

void shader_parse_from_ini_section(Shader *sh, HglIniSection *s)
{
    /* reset shader */
    array_clear(&sh->uniforms);
    array_clear(&sh->shader_depends);

    /* parse name + source + etc. */ 
    sh->name = hgl_sv_from_cstr(s->name);
    const char *tmp = hgl_ini_get_in_section(s, "source");
    if (tmp == NULL) {
        printf("Shader %s: missing `source` entry.\n", s->name);
        sh->src = hgl_sv_from(NULL, 0);
    } else {
        sh->src = hgl_sv_from_cstr(tmp);
    }

    /* parse uniforms */ 
    hgl_ini_reset_kv_pair_iterator(s);
    u32 uniform_idx = 0; 
    while (true) {
        HglIniKVPair *kv = hgl_ini_next_kv_pair(s);
        if(kv == NULL) {
            break;
        }
        if (uniform_parse_from_ini_kv_pair(&sh->uniforms.arr[uniform_idx], kv) == 0) {
            uniform_idx++;
        }
    }
    sh->uniforms.count = uniform_idx;
}

void shader_determine_dependencies(Shader *s)
{
    printf(HGL_SV_FMT "\n", HGL_SV_ARG(s->name));
    printf("%zu\n", s->uniforms.count);

    array_clear(&s->shader_depends);
    for (u32 i = 0; i < s->uniforms.count; i++) {
        Uniform *u = &s->uniforms.arr[i];
        if (u->type != TYPE_TEXTURE) {
            continue;
        }
        SelValue r = sel_run(u->exe);
        if (r.val_tex.kind == 0) {
            array_push(&s->shader_depends, r.val_tex.render_texture_index);
        }
    } 

    printf("[" HGL_SV_FMT "] deps: ", HGL_SV_ARG(s->name));
    for (u32 i = 0; i < s->shader_depends.count; i++) {
        printf("%u ", s->shader_depends.arr[i]);
    }
    printf("\n");

}

/*--- Private functions -----------------------------------------------------------------*/

