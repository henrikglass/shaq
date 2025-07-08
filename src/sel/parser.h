#ifndef PARSER_H
#define PARSER_H

/*--- Include files ---------------------------------------------------------------------*/

#include "sel/ast.h"
#include "sel/lexer.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

Expr *parse_expr(Lexer *l);
void print_expr(Expr *e);
void print_expr_tree(Expr *e);

#endif /* PARSER_H */

