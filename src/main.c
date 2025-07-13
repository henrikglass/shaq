#include <stdio.h>

#include "sel.h"

int main(void)
{
    ExeExpr *e = sel_compile("sin(0.5*PI)");
    SelValue r = sel_run(e);

    switch (e->type) {
        case TYPE_BOOL:  printf("result = %d\n", r.val_bool); break;
        case TYPE_INT:   printf("result = %d\n", r.val_i32); break;
        case TYPE_FLOAT: printf("result = %f\n", (double)r.val_f32); break;
        case TYPE_NIL:   // TODO
        //case TYPE_BVEC2: // TODO
        //case TYPE_BVEC3: // TODO
        //case TYPE_BVEC4: // TODO
        case TYPE_VEC2:  // TODO
        case TYPE_VEC3:  // TODO
        case TYPE_VEC4:  // TODO
        case TYPE_IVEC2: // TODO
        case TYPE_IVEC3: // TODO
        case TYPE_IVEC4: // TODO
        case TYPE_MAT2:  // TODO
        case TYPE_MAT3:  // TODO
        case TYPE_MAT4:  // TODO
        //case TYPE_IMAGE: // TODO
        case TYPE_AND_NAMECHECKER_ERROR_:
        case N_TYPES:
            assert(false);
    }
}

// TODO SEL: Better error handling & error messages
// TODO Remove alloc.h/.c once the program structure is more coherent
