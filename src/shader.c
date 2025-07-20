/*--- Include files ---------------------------------------------------------------------*/

#include "shader.h"

#include "io.h"

#include <errno.h>
#include <string.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

i32 shader_parse_from_ini_section(Shader *sh, HglIniSection *s)
{
    /* reset shader */
    array_clear(&sh->uniforms);
    array_clear(&sh->shader_depends);

    /* parse name + source + etc. */ 
    sh->name = sv_from_cstr(s->name);
    const char *tmp = hgl_ini_get_in_section(s, "source");
    if (tmp == NULL) {
        fprintf(stderr, "[SHAQ] Error: Shader %s: missing `source` entry.\n", s->name);
        return -1;
    } else {
        sh->filepath = sv_from_cstr(tmp);
    }

    /* load source. */ 
    sh->source_code = io_read_entire_file(sh->filepath.start, &sh->source_code_size); // Ok, since sh->filepath was created from a cstr.
    if (sh->source_code == NULL) {
        fprintf(stderr, "[SHAQ] Error: Shader %s: unable to load source file `" HGL_SV_FMT "`. "
                "Errno = %s.\n", s->name, HGL_SV_ARG(sh->filepath), strerror(errno));
        return -1;
    }
    sh->modifytime = io_get_file_modify_time(sh->filepath.start); 
    if (sh->modifytime == -1) {
        fprintf(stderr, "[SHAQ] Error: Shader %s: unable to get modifytime for `" HGL_SV_FMT "`. "
                "Errno = %s.\n", s->name, HGL_SV_ARG(sh->filepath), strerror(errno));
        return -1;
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

    return 0;
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

b8 shader_needs_reload(Shader *s)
{
    i64 modifytime = io_get_file_modify_time(s->filepath.start); 
    if (s->modifytime == -1) {
        return false; // File was probably moved, deleted, or in the processs of being modified. 
                      // Don't immediately ruin everything for the user.
    }
    return modifytime != s->modifytime;
}

/*--- Private functions -----------------------------------------------------------------*/

