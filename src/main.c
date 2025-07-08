#include <stdio.h>

#include "alloc.h"
#include "lexer.h"
#include "parser.h"

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
    
    Lexer l;
    Expr *e;
    l = lexer_begin("sin(2*PI*time())");
    e = parse_expr(&l);
    print_expr(e);
    printf("\n");
    print_expr_tree(e);
    print_usage(temp_allocator);

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
