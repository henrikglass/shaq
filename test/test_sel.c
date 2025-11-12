#include <stdio.h>

#include "sel.h"

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;

    ExeExpr *e = sel_compile(argv[1]);
    if (e == NULL) return 2;
    printf("type = %d\n", e->type);
    printf("qual = %d\n", e->qualifier);
    SelValue r = sel_eval(e, SEL_EMPTY_SVM_CONTEXT, false);
    sel_print_value(e->type, r);
}
