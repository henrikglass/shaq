#ifndef LEXER_H
#define LEXER_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>
#include "hgl_string.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    // TODO Rename to TOK_*
    LPAREN,
    RPAREN,
    PLUS,
    MINUS,
    STAR,
    FSLASH,
    PERCENT,
    COMMA,
    INT_LITERAL,
    FLOAT_LITERAL,
    IDENTIFIER,
    EOF_TOKEN_,
    LEXER_ERROR_,
    NO_TOKEN_,
    N_TOKENS_,
} TokenKind;

typedef struct
{
    TokenKind kind;
    HglStringView text; 
} Token;

typedef struct
{
    HglStringView buf; 
} Lexer;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

Lexer lexer_begin(const char *str);
Token lexer_next(Lexer *l);
void lexer_eat(Lexer *l);
void lexer_expect(Lexer *l, TokenKind kind);
Token lexer_peek(Lexer *l);

void token_print(Token *t);

#endif /* LEXER_H */

