#include <stdio.h>

#include "alloc.h"
#include "sel/parser.h"
#include "sel/typecheck.h"
#include "sel/codegen.h"
#include "sel/eval.h"

int main(void)
{
    //Lexer lexer = lexer_begin("(1+2.23)*sin(time())");
    //while (true) {
    //    Token t = lexer_next(&lexer);

    //    token_print(&t); 

    //    if (t.kind == LEXER_ERROR_) {
    //        printf("lexer error\n");
    //        break;
    //    }

    //    if (t.kind == EOF_TOKEN_) {
    //        printf("EOF\n");
    //        break;
    //    }
    //}
    
    Type t;
    Lexer l;
    Expr *e;
    EExpr ee;
    ExeCtx ectx;
    //l = lexer_begin("sin(2*PI*time())");
    //l = lexer_begin("sin(4.0 * time())");
    l = lexer_begin("4.0*5.0+40.+9.0");
    e = parse_expr(&l);
    t = typecheck(e);
    ee = codegen(e);
    ectx = make_execution_context(&ee);
    f32 *res = (f32 *)eval(&ectx); 
    printf("TYPE IS `%s`\n", TYPE_TO_STR[t]);
    printf("result: %f\n", (f64) *res);


    print_expr(e);
    printf("\n");
    print_expr_tree(e);

    //l = lexer_begin("1+2+3-4-5+6 ");
    //e = parse_expr(&l);
    //print_expr(e);
    //printf("\n");

    //l = lexer_begin("1+2*3");
    //e = parse_expr(&l);
    //print_expr(e);
    //printf("\n");

    //l = lexer_begin("1*2+3 ");
    //e = parse_expr(&l);
    //print_expr(e);
    //printf("\n");

    //l = lexer_begin("5-4+3*2/1 ");
    //e = parse_expr(&l);
    //print_expr(e);
    //printf("\n");
}


// TODO packa ihop SEL + funktioner
// TODO SEL: Compile (flattened prefix expressions?)
// TODO Remove alloc.h/.c once the program structure is more coherent
// TODO SEL: Better error handling & error messages
