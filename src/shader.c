/*--- Include files ---------------------------------------------------------------------*/

#include "shader.h"
#include "io.h"
#include "glad/glad.h"
#include "shaq_core.h"

#include <errno.h>
#include <string.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

u32 make_shader_program(u8 *frag_shader_src);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

const char *const PASS_THROUGH_VERT_SHADER_SOURCE =
    "#version 450 core\n                        "
    "\n                                         "
    "layout (location = 0) in vec2 in_xy;\n     "
    "\n                                         "
    "void main(void)\n                          "
    "{\n                                        "
    "    gl_Position = vec4(in_xy, 0.0, 1.0);\n "
    "}\n                                        "
;

const char *const LAST_PASS_FRAGMENT_SHADER_SOURCE =
    "#version 450 core\n                          "
    "\n                                           "
    "out vec4 frag_color;\n                       "
    "\n                                           "
    "void main(void)\n                            "
    "{\n                                          "
    "    frag_color = vec4(0.2, 0.5, 0.2, 1.0);\n "
    "}\n                                          "
;

/*--- Public functions ------------------------------------------------------------------*/

i32 shader_parse_from_ini_section(Shader *sh, HglIniSection *s)
{
    /* reset shader */
    memset(sh, 0, sizeof(*sh));
    array_clear(&sh->uniforms); // not necessary
    array_clear(&sh->shader_depends); // not necessary

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
    sh->frag_shader_src = io_read_entire_file(sh->filepath.start, &sh->frag_shader_src_size); // Ok, since sh->filepath was created from a cstr.
    if (sh->frag_shader_src == NULL) {
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
        SelValue r = sel_eval(u->exe, true);
        if (r.val_tex.kind == SHADER_INDEX) {
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

void shader_reload(Shader *s)
{
    if (s->gl_shader_program_id != 0) {
        glDeleteProgram(s->gl_shader_program_id);
        s->gl_shader_program_id = 0;
    }
    texture_free_opengl_resources(&s->render_texture);

    u32 vert_shader = glCreateShader(GL_VERTEX_SHADER);
    u32 frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    u32 shader_program = glCreateProgram();

    i32 size = (i32) s->frag_shader_src_size;
    glShaderSource(vert_shader, 1, &PASS_THROUGH_VERT_SHADER_SOURCE, NULL);
    glShaderSource(frag_shader, 1, (const char * const *)&s->frag_shader_src, &size);
    glCompileShader(vert_shader);
    glCompileShader(frag_shader);
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);
    
    i32 vert_success, frag_success, link_success;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &vert_success);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &frag_success);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &link_success);

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    // TODO better error handling
    if (!(vert_success & frag_success & link_success))
    {
        char log[512];
        glGetShaderInfoLog(vert_shader, 512, NULL, log);
        fprintf(stderr, "Vertex shader error: %s\n", log);
        glGetShaderInfoLog(frag_shader, 512, NULL, log);
        fprintf(stderr, "Fragment shader error: %s\n", log);
        glGetProgramInfoLog(shader_program, 512, NULL, log);
        fprintf(stderr, "Linking error: %s\n", log);
        return;
    }

    s->gl_shader_program_id = shader_program;

    s->render_texture = texture_make_empty();

    glUseProgram(s->gl_shader_program_id); // necessary?
    for (u32 i = 0; i < s->uniforms.count; i++) {
        Uniform *u = &s->uniforms.arr[i];
        uniform_determine_location_in_shader_program(u, s->gl_shader_program_id);
    }
}

void shader_make_last_pass_shader(Shader *s)
{
    u32 vert_shader = glCreateShader(GL_VERTEX_SHADER);
    u32 frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    u32 shader_program = glCreateProgram();

    glShaderSource(vert_shader, 1, &PASS_THROUGH_VERT_SHADER_SOURCE, NULL);
    glShaderSource(frag_shader, 1, &LAST_PASS_FRAGMENT_SHADER_SOURCE, NULL);
    glCompileShader(vert_shader);
    glCompileShader(frag_shader);
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);
    
    i32 vert_success, frag_success, link_success;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &vert_success);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &frag_success);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &link_success);

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    // TODO better error handling
    if (!(vert_success & frag_success & link_success))
    {
        char log[512];
        glGetShaderInfoLog(vert_shader, 512, NULL, log);
        fprintf(stderr, "Vertex shader error: %s\n", log);
        glGetShaderInfoLog(frag_shader, 512, NULL, log);
        fprintf(stderr, "Fragment shader error: %s\n", log);
        glGetProgramInfoLog(shader_program, 512, NULL, log);
        fprintf(stderr, "Linking error: %s\n", log);
        return;
    }

    s->name = HGL_SV_LIT("LAST-PASS");
    s->gl_shader_program_id = shader_program;
    s->uniforms.count = 0;
    s->shader_depends.count = 0;
}

void shader_free_opengl_resources(Shader *s)
{
    if (s->gl_shader_program_id != 0) {
        glDeleteProgram(s->gl_shader_program_id);
    }
    glDeleteTextures(1, &s->render_texture.gl_texture_id);
}

void shader_prepare_for_drawing(Shader *s)
{
    if (s->gl_shader_program_id == 0) {
        return;
    }
    glUseProgram(s->gl_shader_program_id);

    u32 texture_unit = 0;

    for (u32 i = 0; i < s->uniforms.count; i++) {
        Uniform *u = &s->uniforms.arr[i];
        if (u->exe == NULL) {
            continue;
        }
        if (u->gl_uniform_location == -1) {
            continue;
        }

        SelValue r = sel_eval(u->exe, false);
        //if (u->exe->qualifier == QUALIFIER_CONST) // TODO
        switch (u->type) {
            case TYPE_BOOL:  {glUniform1i(u->gl_uniform_location,  r.val_bool);} break;
            case TYPE_INT:   {glUniform1i(u->gl_uniform_location,  r.val_i32);} break;
            case TYPE_UINT:  {glUniform1ui(u->gl_uniform_location, r.val_u32);} break;
            case TYPE_FLOAT: {glUniform1f(u->gl_uniform_location,  r.val_f32);} break;
            case TYPE_VEC2:  {glUniform2fv(u->gl_uniform_location, 1, (f32 *)&r.val_vec2);} break;
            case TYPE_VEC3:  {glUniform3fv(u->gl_uniform_location, 1, (f32 *)&r.val_vec3);} break;
            case TYPE_VEC4:  {glUniform4fv(u->gl_uniform_location, 1, (f32 *)&r.val_vec4);} break;
            case TYPE_IVEC2: {glUniform2iv(u->gl_uniform_location, 1, (i32 *)&r.val_ivec2);} break;
            case TYPE_IVEC3: {glUniform3iv(u->gl_uniform_location, 1, (i32 *)&r.val_ivec3);} break;
            case TYPE_IVEC4: {glUniform4iv(u->gl_uniform_location, 1, (i32 *)&r.val_ivec4);} break;
            case TYPE_MAT2:  {glUniformMatrix2fv(u->gl_uniform_location, 1, false, (f32 *)&r.val_mat2);} break;
            case TYPE_MAT3:  {glUniformMatrix3fv(u->gl_uniform_location, 1, false, (f32 *)&r.val_mat3);} break;
            case TYPE_MAT4:  {glUniformMatrix4fv(u->gl_uniform_location, 1, false, (f32 *)&r.val_mat4);} break;
            case TYPE_TEXTURE: {
                TextureIndex idx = r.val_tex;
                glActiveTexture(GL_TEXTURE0 + texture_unit);
                glUniform1i(u->gl_uniform_location, texture_unit);
                texture_unit++;

                if (idx.kind == SHADER_INDEX) {
                    glBindTexture(GL_TEXTURE_2D, shaq_get_shader_render_texture_by_index(idx.render_texture_index));
                } else if (idx.kind == LOADED_TEXTURE_INDEX) {
                    glBindTexture(GL_TEXTURE_2D, shaq_get_loaded_texture_by_index(idx.loaded_texture_index));
                } 
            } break; 
            case TYPE_STR:
            case TYPE_NIL:
            case TYPE_AND_NAMECHECKER_ERROR_:
            case N_TYPES:
                fprintf(stderr, "whoopsie... logic error...\n");
        }
    }
}

void shader_draw(Shader *s)
{
    if (s->gl_shader_program_id == 0) {
        return;
    }
    glUseProgram(s->gl_shader_program_id);
}


/*--- Private functions -----------------------------------------------------------------*/


