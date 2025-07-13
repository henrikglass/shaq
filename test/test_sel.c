#include <stdio.h>

#include "sel.h"

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;

    ExeExpr *e = sel_compile(argv[1]);
    if (e == NULL) return 2;
    printf("type = %d\n", e->type);
    SelValue r = sel_run(e);

    switch (e->type) {
        case TYPE_BOOL:  printf("result = %s\n", r.val_bool ? "true" : "false"); break;
        case TYPE_INT:   printf("result = %d\n", r.val_i32); break;
        case TYPE_FLOAT: printf("result = %f\n", (double)r.val_f32); break;
        //case TYPE_BVEC2: // TODO
        //case TYPE_BVEC3: // TODO
        //case TYPE_BVEC4: // TODO
        case TYPE_VEC2:  vec2_print(r.val_vec2); break;
        case TYPE_VEC3:  vec3_print(r.val_vec3); break;
        case TYPE_VEC4:  vec4_print(r.val_vec4); break;
        case TYPE_IVEC2: ivec2_print(r.val_ivec2); break;
        case TYPE_IVEC3: ivec3_print(r.val_ivec3); break;
        case TYPE_IVEC4: ivec4_print(r.val_ivec4); break;
        case TYPE_MAT2:  mat2_print(r.val_mat2); break;
        case TYPE_MAT3:  mat3_print(r.val_mat3); break;
        case TYPE_MAT4:  mat4_print(r.val_mat4); break;
        //case TYPE_IMAGE: // TODO
        case TYPE_NIL:   // TODO
        case TYPE_AND_NAMECHECKER_ERROR_:
        case N_TYPES:
            assert(false);
    }
}
