#include <stdio.h>

#include "sel.h"
#include "alloc.h"
#include "log.h"

int main(int argc, char *argv[])
{
    log_set_mode(LOG_R2R);
    alloc_init();

    if (argc < 2) return 1;

    ExeExpr *e = sel_compile(argv[1], g_r2r_arena);
    log_print();
    if (e == NULL) return 2;
    printf("type = %d\n", e->type);
    printf("qual = %d\n", e->qualifier);
    SelValue r = sel_eval(e, SEL_EMPTY_SVM_CONTEXT, false);
    sel_print_value(e->type, r);
    log_print();
    printf("\n");
}
