/*--- Include files ---------------------------------------------------------------------*/

#include "uniform.h"

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

i32 uniform_parse_from_ini_kv_pair(Uniform *u, HglIniKVPair *kv)
{
    StringView k = sv_trim(sv_from_cstr(kv->key));

    if (sv_starts_with_lchop(&k, "uniform")) {
        k = sv_ltrim(k);
        if      (sv_starts_with_lchop(&k, "bool"))      { u->type = TYPE_BOOL;    }
        else if (sv_starts_with_lchop(&k, "int"))       { u->type = TYPE_INT;     }
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
            printf("Unknown or unsupported type in lhs expression: `%s`.\n", kv->key);
            return -1;
        }

        u->name = sv_trim(k);
        u->exe = sel_compile(kv->val);

        if (u->exe == NULL) {
            fprintf(stderr, "[SHAQ] Error: Could not compile expression: `%s`.\n", kv->val);
            return -1;
        }

        if (u->exe->type != u->type) {
            fprintf(stderr, "[SHAQ] Error: The expression `%s` has type `%s` which does not match the specified "
                    "uniform type `%s`.\n", kv->val, TYPE_TO_STR[u->exe->type], TYPE_TO_STR[u->type]);
            return -1;
        }

        return 0;
    }
    
    return -1;
}

/*--- Private functions -----------------------------------------------------------------*/

