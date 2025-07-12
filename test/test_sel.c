#include <stdio.h>

#include "sel.h"

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;

    ExeExpr *e = sel_compile(argv[1]);
    SelValue r = sel_run(e);

    switch (e->type) {
        case TYPE_BOOL:  printf("result = %d\n", r.val_bool); break;
        case TYPE_INT:   printf("result = %d\n", r.val_i32); break;
        case TYPE_FLOAT: printf("result = %f\n", (double)r.val_f32); break;
        case TYPE_NIL:   // TODO
        case TYPE_BVEC2: // TODO
        case TYPE_BVEC3: // TODO
        case TYPE_BVEC4: // TODO
        case TYPE_VEC2:  // TODO
        case TYPE_VEC3:  // TODO
        case TYPE_VEC4:  // TODO
        case TYPE_IVEC2: // TODO
        case TYPE_IVEC3: // TODO
        case TYPE_IVEC4: // TODO
        case TYPE_MAT2:  // TODO
        case TYPE_MAT3:  // TODO
        case TYPE_MAT4:  // TODO
        case TYPE_IMAGE: // TODO
        case TYPE_ERROR_:
        case NAME_ERROR_:
        case N_TYPES:
            assert(false);
    }
}
