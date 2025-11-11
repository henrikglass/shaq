/*--- Include files ---------------------------------------------------------------------*/

#include "shader.h"
#include "io.h"
#include "glad/glad.h"
#include "shaq_core.h"
#include "renderer.h"
#include "gui.h"
#include "log.h"

#include <errno.h>
#include <string.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

u32 make_shader_program(u8 *frag_shader_src); // TODO remove?
static void parse_attribute_from_kv_pair(Shader *s, HglIniKVPair *kv);
static size_t whitespace_lexeme(StringView sv);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

const char *const PASS_THROUGH_VERT_SHADER_SOURCE =
    "#version 330 core\n                        "
    "\n                                         "
    "layout (location = 0) in vec2 in_xy;\n     "
    "\n                                         "
    "void main(void)\n                          "
    "{\n                                        "
    "    gl_Position = vec4(in_xy, 0.0, 1.0);\n "
    "}\n                                        "
;

const char *const LAST_PASS_FRAGMENT_SHADER_SOURCE =
    "#version 330 core\n                                    "
    "\n                                                     "
    "out vec4 frag_color;\n                                 "
    "\n                                                     "
    "uniform sampler2D tex;\n                               "
    "uniform ivec2 iresolution;\n                           "
    "\n                                                     "
    "void main(void)\n                                      "
    "{\n                                                    "
    "    vec2 uv = gl_FragCoord.xy / iresolution;\n         "
    "    frag_color = vec4(texture(tex, uv).rgb, 1.0);\n    "
    "}\n                                                    "
;

/*--- Public functions ------------------------------------------------------------------*/

i32 shader_parse_from_ini_section(Shader *sh, HglIniSection *s)
{
    /* reset shader */
    memset(sh, 0, sizeof(*sh));
    array_clear(&sh->uniforms); // not necessary
    array_clear(&sh->shader_depends); // not necessary

    /* get name */ 
    sh->name = sv_from_cstr(s->name);

    /* parse key-value pairs */ 
    hgl_ini_reset_kv_pair_iterator(s);
    u32 uniform_idx = 0; 
    while (true) {
        HglIniKVPair *kv = hgl_ini_next_kv_pair(s);
        if(kv == NULL) {
            break;
        }
        StringView key = sv_trim(sv_from_cstr(kv->key));
        if (sv_starts_with(&key, "uniform")) {
            int err = uniform_parse_from_ini_kv_pair(&sh->uniforms.arr[uniform_idx], kv);
            if (err == 0) {
                uniform_idx++;
            }
        } else if (sv_starts_with(&key, "attribute")) {
            parse_attribute_from_kv_pair(sh, kv);
        } else {
            log_error("Expected either `attribute` or `uniform` for key-value pair `%s = %s`.", kv->key, kv->val);
        }
    }
    sh->uniforms.count = uniform_idx;

    /* Assert all necessary attributes have been specified. */ 
    if (sh->attributes.source.start == NULL) {
        log_error("Shader \"%s\" is missing a `source` entry.", s->name);
        return -1;
    }

    /* load fragment shader source. */ 
    sh->frag_shader_src = io_read_entire_file(g_r2r_fs_allocator, sh->attributes.source.start, &sh->frag_shader_src_size); // Ok, since sh->source was created from a cstr.
    if (sh->frag_shader_src == NULL) {
        log_error("Shader \"%s\": unable to load source file `" SV_FMT "`. "
                  "Errno = %s.", s->name, SV_ARG(sh->attributes.source), strerror(errno));
        return -1;
    }

    /* update modify time. */ 
    sh->modifytime = io_get_file_modify_time(sh->attributes.source.start, true); 
    if (sh->modifytime == -1) {
        log_error("Shader \"%s\": unable to get modifytime for `" SV_FMT "`. "
                  "Errno = %s.", s->name, SV_ARG(sh->attributes.source), strerror(errno));
        return -1;
    }

    /* if output_resolution && output_format are unspecified, give them default values */ 
    if (sh->attributes.output_format == 0) {
        sh->attributes.output_format = GL_RGBA;
    }
    if (sh->attributes.output_resolution.x == 0 &&
        sh->attributes.output_resolution.y == 0) {
        sh->attributes.output_resolution = renderer_shader_viewport_size();
    } 

    return 0;
}

void shader_determine_dependencies(Shader *s)
{
    array_clear(&s->shader_depends);
    for (u32 i = 0; i < s->uniforms.count; i++) {
        Uniform *u = &s->uniforms.arr[i];
        if (u->type != TYPE_TEXTURE) {
            continue;
        }
        SelValue r = sel_eval(u->exe, true);
        if (r.val_tex.kind == SHADER_CURRENT_RENDER_TEXTURE) {
            array_push(&s->shader_depends, r.val_tex.texture_index);
        }
    } 
    for (u32 i = 0; i < s->attributes.render_after.count; i++) {
        StringView sv = s->attributes.render_after.arr[i]; 
        i32 sid = shaq_find_shader_id_by_name(sv);
        if (sid == -1) {
            log_error("Shader \"" SV_FMT "\": In explicit `render_after` attribute - No such shader: \"" SV_FMT "\"", 
                      SV_ARG(s->name), SV_ARG(sv));
        } else {
            array_push(&s->shader_depends, sid);
        }
    }
}

b8 shader_was_modified(Shader *s)
{
    i64 modifytime = io_get_file_modify_time(s->attributes.source.start, true); 
    if (s->modifytime == -1) {
        return false; // File was probably moved, deleted, or in the processs of being modified. 
                      // Don't immediately ruin everything for the user.
    }
    return modifytime != s->modifytime;
}

b8 shader_is_ok(const Shader *s)
{
    if (s == NULL) {
        return false;
    }

    if (s->render_texture_current == NULL) {
        return false;
    }

    if (s->render_texture_last == NULL) {
        return false;
    }

    return true;
}

void shader_reload(Shader *s)
{
    if (s->gl_shader_program_id != 0) {
        glDeleteProgram(s->gl_shader_program_id);
        s->gl_shader_program_id = 0;
    }
    texture_free(&s->render_texture[0]);
    texture_free(&s->render_texture[1]);

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
        char log[4096];
        log_error("Failed to compile shader `" SV_FMT "`.", SV_ARG(s->name));
        glGetShaderInfoLog(vert_shader, 4096, NULL, log);
        log_error("Vertex shader error(s):\n%s", log);
        glGetShaderInfoLog(frag_shader, 4096, NULL, log);
        log_error("Fragment shader error(s):\n%s", log);
        glGetProgramInfoLog(shader_program, 4096, NULL, log);
        log_error("Linking error(s):\n%s", log);
        return;
    }

    s->gl_shader_program_id = shader_program;
    s->render_texture[0] = texture_make_empty(s->attributes.output_resolution, 
                                              s->attributes.output_format);
    s->render_texture[1] = texture_make_empty(s->attributes.output_resolution, 
                                              s->attributes.output_format);
    s->render_texture_current = &s->render_texture[0];
    s->render_texture_last = &s->render_texture[1];

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
        char log[4096];
        log_error("Failed to compile shader `" SV_FMT "`.", SV_ARG(s->name));
        glGetShaderInfoLog(vert_shader, 4096, NULL, log);
        log_error("Vertex shader error(s):\n%s", log);
        glGetShaderInfoLog(frag_shader, 4096, NULL, log);
        log_error("Fragment shader error(s):\n%s", log);
        glGetProgramInfoLog(shader_program, 4096, NULL, log);
        log_error("Linking error(s):\n%s", log);
        return;
    }

    s->name = SV_LIT("LAST-PASS");
    s->gl_shader_program_id = shader_program;
    s->uniforms.count = 0;
    s->shader_depends.count = 0;
}

void shader_free_opengl_resources(Shader *s)
{
    if (s->gl_shader_program_id != 0) {
        glDeleteProgram(s->gl_shader_program_id);
    }
    glDeleteTextures(1, &s->render_texture[0].gl_texture_id);
    glDeleteTextures(1, &s->render_texture[1].gl_texture_id);
}

void shader_swap_render_textures(Shader *s)
{
    Texture *temp = s->render_texture_current;
    s->render_texture_current = s->render_texture_last;
    s->render_texture_last = temp;
}

void shader_update_uniforms(Shader *s)
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

        switch (u->type) {
            case TYPE_BOOL:  glUniform1i(u->gl_uniform_location,  r.val_bool); break;
            case TYPE_INT:   glUniform1i(u->gl_uniform_location,  r.val_i32); break;
            case TYPE_UINT:  glUniform1ui(u->gl_uniform_location, r.val_u32); break;
            case TYPE_FLOAT: glUniform1f(u->gl_uniform_location,  r.val_f32); break;
            case TYPE_VEC2:  glUniform2fv(u->gl_uniform_location, 1, (f32 *)&r.val_vec2); break;
            case TYPE_VEC3:  glUniform3fv(u->gl_uniform_location, 1, (f32 *)&r.val_vec3); break;
            case TYPE_VEC4:  glUniform4fv(u->gl_uniform_location, 1, (f32 *)&r.val_vec4); break;
            case TYPE_IVEC2: glUniform2iv(u->gl_uniform_location, 1, (i32 *)&r.val_ivec2); break;
            case TYPE_IVEC3: glUniform3iv(u->gl_uniform_location, 1, (i32 *)&r.val_ivec3); break;
            case TYPE_IVEC4: glUniform4iv(u->gl_uniform_location, 1, (i32 *)&r.val_ivec4); break;
            case TYPE_MAT2:  glUniformMatrix2fv(u->gl_uniform_location, 1, false, (f32 *)&r.val_mat2); break;
            case TYPE_MAT3:  glUniformMatrix3fv(u->gl_uniform_location, 1, false, (f32 *)&r.val_mat3); break;
            case TYPE_MAT4:  glUniformMatrix4fv(u->gl_uniform_location, 1, false, (f32 *)&r.val_mat4); break;
            case TYPE_TEXTURE: {
                TextureDescriptor desc = r.val_tex;
                glActiveTexture(GL_TEXTURE0 + texture_unit);
                glUniform1i(u->gl_uniform_location, texture_unit);
                texture_unit++;

                u32 texture_id = 0;
                switch(desc.kind) {
                    case SHADER_CURRENT_RENDER_TEXTURE: {
                        texture_id = shaq_get_shader_current_render_texture_by_shader_id(desc.texture_index);
                    } break;

                    case SHADER_LAST_RENDER_TEXTURE: {
                        texture_id = shaq_get_shader_last_render_texture_by_shader_id(desc.texture_index);
                    } break;

                    case LOADED_TEXTURE: {
                        texture_id = shaq_get_loaded_texture_by_texture_id(desc.texture_index);
                    } break;
                }
                if (texture_id != 0) {
                    glBindTexture(GL_TEXTURE_2D, texture_id);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, desc.filter);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, desc.filter);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, desc.wrap);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, desc.wrap);
                }
            } break; 
            case TYPE_STR:
            case TYPE_NIL:
            case TYPE_AND_NAMECHECKER_ERROR_:
            case N_TYPES:
                log_error("Strange logic error that shouldn't happen<%s:%d>", __FILE__, __LINE__);
        }
    }
}

Uniform *shader_find_uniform_by_name(Shader *s, StringView name)
{
    for (u32 i = 0; i < s->uniforms.count; i++) {
        Uniform *u  = &s->uniforms.arr[i];
        if (sv_equals(u->name, name)) {
            return u;
        }
    }

    return NULL;
}

/*--- Private functions -----------------------------------------------------------------*/


static void parse_attribute_from_kv_pair(Shader *s, HglIniKVPair *kv)
{
    StringView k = sv_trim(sv_from_cstr(kv->key));

    if (!sv_starts_with_lchop(&k, "attribute")) {
        log_error("Expected keyword `attribute` in left-hand-side expression: `%s`.", kv->key);
        return;
    }

    /* expect whitespace */
    if (!sv_starts_with_lexeme(&k, whitespace_lexeme)) {
        log_error("Malformed left-hand-side expression: `%s`.", kv->key);
        return;
    }

    k = sv_ltrim(k);
    ExeExpr *exe = sel_compile(kv->val);
    if (exe == NULL) {
        log_error("Could not compile shader attribute expression: `%s`.", kv->val);
        return;
    }

    if ((exe->qualifier & QUALIFIER_CONST) == 0) {
        log_error("Shader attributes `%s` has a non-constant expression `%s`\n", kv->key, kv->val);
        return;
    }

    /* Parse `source` attribute */
    if (sv_starts_with_lchop(&k, "source") && sv_trim(k).length == 0) {
        if (exe->type != TYPE_STR) {
            log_error("Shader `" SV_FMT "`: Attribute `source` attribute must have type `str`.", SV_ARG(s->name));
            return;
        }
        s->attributes.source = sv_make_copy(sel_eval(exe, true).val_str, r2r_fs_alloc);
    } else if (sv_starts_with_lchop(&k, "output_format") && sv_trim(k).length == 0) {
        if (exe->type != TYPE_INT) {
            log_error("Shader `" SV_FMT "`: Attribute `output_format` attribute must have type `int`.", SV_ARG(s->name));
            return;
        }
        s->attributes.output_format = sel_eval(exe, true).val_i32;
    } else if (sv_starts_with_lchop(&k, "output_resolution") && sv_trim(k).length == 0) {
        if (exe->type != TYPE_IVEC2) {
            log_error("Shader `" SV_FMT "`: Attribute `output_resolution` attribute must have type `ivec2`.", SV_ARG(s->name));
            return;
        }
        s->attributes.output_resolution = sel_eval(exe, true).val_ivec2;
    } else if (sv_starts_with_lchop(&k, "render_after") && sv_trim(k).length == 0) {
        if (exe->type != TYPE_STR) {
            log_error("Shader `" SV_FMT "`: Attribute `render_after` attribute must have type `str`.", SV_ARG(s->name));
            return;
        }
        array_push(&s->attributes.render_after, sv_make_copy(sel_eval(exe, true).val_str, r2r_fs_alloc));
    } else {
        log_error("Shader `" SV_FMT "`: Unrecognized attribute `" SV_FMT "`", SV_ARG(s->name), SV_ARG(k));
    }
}

static size_t whitespace_lexeme(StringView sv)
{
    if (sv.length < 1) return 0;
    if (isspace(sv.start[0])) return 1;
    return 0;
}

