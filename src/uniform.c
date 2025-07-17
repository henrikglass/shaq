/*--- Include files ---------------------------------------------------------------------*/

#include "uniform.h"

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

int uniform_parse_from_ini_kv_pair(Uniform *u, HglIniKVPair *kv)
{
    HglStringView k = hgl_sv_trim(hgl_sv_from_cstr(kv->key));

    if (hgl_sv_starts_with_lchop(&k, "uniform")) {
        k = hgl_sv_ltrim(k);
        if      (hgl_sv_starts_with_lchop(&k, "bool"))      { u->type = TYPE_BOOL;    }
        else if (hgl_sv_starts_with_lchop(&k, "int"))       { u->type = TYPE_INT;     }
        else if (hgl_sv_starts_with_lchop(&k, "float"))     { u->type = TYPE_FLOAT;   }
        else if (hgl_sv_starts_with_lchop(&k, "vec2"))      { u->type = TYPE_VEC2;    }
        else if (hgl_sv_starts_with_lchop(&k, "vec3"))      { u->type = TYPE_VEC3;    }
        else if (hgl_sv_starts_with_lchop(&k, "vec4"))      { u->type = TYPE_VEC4;    }
        else if (hgl_sv_starts_with_lchop(&k, "ivec2"))     { u->type = TYPE_IVEC2;   }
        else if (hgl_sv_starts_with_lchop(&k, "ivec3"))     { u->type = TYPE_IVEC3;   }
        else if (hgl_sv_starts_with_lchop(&k, "ivec4"))     { u->type = TYPE_IVEC4;   }
        else if (hgl_sv_starts_with_lchop(&k, "mat2"))      { u->type = TYPE_MAT2;    }
        else if (hgl_sv_starts_with_lchop(&k, "mat3"))      { u->type = TYPE_MAT3;    }
        else if (hgl_sv_starts_with_lchop(&k, "mat4"))      { u->type = TYPE_MAT4;    }
        else if (hgl_sv_starts_with_lchop(&k, "sampler2D")) { u->type = TYPE_TEXTURE; }
        else {
            printf("Unknown or unsupported type in lhs expression: `%s`.\n", kv->key);
            return -1;
        }

        u->name = hgl_sv_trim(k);
        u->exe = sel_compile(kv->val);
        return 0;
    }
    
    return -1;
}

/*--- Private functions -----------------------------------------------------------------*/

