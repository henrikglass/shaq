/*--- Include files ---------------------------------------------------------------------*/

#include "uniform.h"
#include "alloc.h"
#include "log.h"

#include "glad/glad.h"

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static size_t whitespace_lexeme(StringView sv);
static size_t identifier_lexeme(StringView sv);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

i32 uniform_parse_from_ini_kv_pair(Uniform *u, HglIniKVPair *kv)
{
    StringView k = sv_trim(sv_from_cstr(kv->key));

    if (!sv_starts_with_lchop(&k, "uniform")) {
        log_error("Expected keyword `uniform` in left-hand-side expression: `%s`.", kv->key);
        return -1;
    }

    /* expect whitespace */
    if (!sv_starts_with_lexeme(&k, whitespace_lexeme)) {
        log_error("Malformed left-hand-side expression: `%s`.", kv->key);
        return -1;
    }

    /* expect type identifier */
    k = sv_ltrim(k);
    if      (sv_starts_with_lchop(&k, "bool"))      { u->type = TYPE_BOOL;    }
    else if (sv_starts_with_lchop(&k, "int"))       { u->type = TYPE_INT;     }
    else if (sv_starts_with_lchop(&k, "uint"))      { u->type = TYPE_UINT;    }
    else if (sv_starts_with_lchop(&k, "float"))     { u->type = TYPE_FLOAT;   }
    else if (sv_starts_with_lchop(&k, "vec2"))      { u->type = TYPE_VEC2;    }
    else if (sv_starts_with_lchop(&k, "vec3"))      { u->type = TYPE_VEC3;    }
    else if (sv_starts_with_lchop(&k, "vec4"))      { u->type = TYPE_VEC4;    }
    else if (sv_starts_with_lchop(&k, "ivec2"))     { u->type = TYPE_IVEC2;   }
    else if (sv_starts_with_lchop(&k, "ivec3"))     { u->type = TYPE_IVEC3;   }
    else if (sv_starts_with_lchop(&k, "ivec4"))     { u->type = TYPE_IVEC4;   }
    else if (sv_starts_with_lchop(&k, "mat2"))      { u->type = TYPE_MAT2;    }
    else if (sv_starts_with_lchop(&k, "mat3"))      { u->type = TYPE_MAT3;    }
    else if (sv_starts_with_lchop(&k, "mat4"))      { u->type = TYPE_MAT4;    }
    else if (sv_starts_with_lchop(&k, "sampler2D")) { u->type = TYPE_TEXTURE; }
    else {
        log_error("Unknown or unsupported type in left-hand-side expression: `%s`.", kv->key);
        return -1;
    }

    /* expect whitespace */
    if (!sv_starts_with_lexeme(&k, whitespace_lexeme)) {
        log_error("Malformed left-hand-side expression: `%s`.", kv->key);
        return -1;
    }

    /* expect type identifier */
    k = sv_ltrim(k);
    u->name = sv_lchop_lexeme(&k, identifier_lexeme);
    if (u->name.length == 0) {
        log_error("Malformed left-hand-side expression: `%s`.", kv->key);
        return -1;
    }

    u->exe = sel_compile(kv->val);

    if (u->exe == NULL) {
        log_error("Could not compile expression: `%s`.", kv->val);
        return -1;
    }

    if (u->exe->type != u->type) {
        log_error("The expression `%s` has type `%s` which does not match the specified uniform type `%s`.", 
                  kv->val, TYPE_TO_STR[u->exe->type], TYPE_TO_STR[u->type]);
        return -1;
    }

    return 0;
}

void uniform_map_shader_uniform(Uniform *u, u32 shader_program)
{
    static const i32 sel_to_gl_type[N_TYPES] = {
        [TYPE_BOOL]    = GL_BOOL,
        [TYPE_INT]     = GL_INT,
        [TYPE_UINT]    = GL_UNSIGNED_INT,
        [TYPE_FLOAT]   = GL_FLOAT,
        [TYPE_VEC2]    = GL_FLOAT_VEC2,
        [TYPE_VEC3]    = GL_FLOAT_VEC3,
        [TYPE_VEC4]    = GL_FLOAT_VEC4,
        [TYPE_IVEC2]   = GL_INT_VEC2,
        [TYPE_IVEC3]   = GL_INT_VEC3,
        [TYPE_IVEC4]   = GL_INT_VEC4,
        [TYPE_MAT2]    = GL_FLOAT_MAT2,
        [TYPE_MAT3]    = GL_FLOAT_MAT3,
        [TYPE_MAT4]    = GL_FLOAT_MAT4,
        [TYPE_TEXTURE] = GL_SAMPLER_2D,
    };

    u->gl_uniform_location = -1;
    u32 index = GL_INVALID_INDEX;
    i32 type = -1;
    const char *name_cstr = hgl_sv_make_cstr_copy(u->name, tmp_alloc);
    u->gl_uniform_location = glGetUniformLocation(shader_program, name_cstr);
    if (u->gl_uniform_location == -1) {
        log_error("Could not locate uniform variable: `%s`.", name_cstr);
    }
    glGetUniformIndices(shader_program, 1, &name_cstr, &index);
    if (index == GL_INVALID_INDEX) {
        log_error("Could not query index of uniform variable: `%s`.", name_cstr);
        u->gl_uniform_location = -1;
        return;
    }
    glGetActiveUniformsiv(shader_program, 1, &index, GL_UNIFORM_TYPE, &type);
    if (sel_to_gl_type[u->type] != type) {
        log_error("The specified type `%s` of uniform `%s` does not match its actual type.", 
                  TYPE_TO_STR[u->type], name_cstr);
        u->gl_uniform_location = -1;
        return;
    }
}

/*--- Private functions -----------------------------------------------------------------*/

static size_t whitespace_lexeme(StringView sv)
{
    if (sv.length < 1) return 0;
    if (isspace(sv.start[0])) return 1;
    return 0;
}

static size_t identifier_lexeme(StringView sv)
{
    size_t i, j;

    for (i = 0; i < sv.length; i++) {
        if (!isalnum(sv.start[i]) && sv.start[i] != '_') {
            break;
        } 
    }

    for (j = i; j < sv.length; j++) {
        if (!isspace(sv.start[j])) {
            break;
        } 
    }

    if (j != sv.length) return 0; // identifier trailed by something other than whitespace
    return i;
}

