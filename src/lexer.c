/*--- Include files ---------------------------------------------------------------------*/

#include "lexer.h"

#define HGL_STRING_IMPLEMENTATION
#include "hgl_string.h"

#include <assert.h>

/*--- Private macros --------------------------------------------------------------------*/

#define LEXER_ASSERT(cond, ...)                    \
    do {                                           \
        if (!(cond)) {                             \
            return (Token) {.kind = LEXER_ERROR_}; \
        }                                          \
    } while(0)

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static bool is_identifier_char(int c);
static bool is_decimal_char(int c);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

Lexer lexer_begin(const char *str)
{
    Lexer l = {0};
    l.buf = hgl_sv_from_cstr(str);
    return l;
}

Token lexer_next(Lexer *l)
{
    Token t = lexer_peek(l);
    hgl_sv_lchop(&l->buf, t.text.length);
    return t;
}

void lexer_eat(Lexer *l)
{
    (void) lexer_next(l);
}

void lexer_expect(Lexer *l, TokenKind kind)
{
    assert(lexer_next(l).kind == kind);
}

Token lexer_peek(Lexer *l)
{
    l->buf = hgl_sv_ltrim(l->buf);

    if (l->buf.length == 0) {
        return (Token) {.kind = EOF_TOKEN_};
    }

    char c = l->buf.start[0];
    switch (c) {
        case '\0': return (Token) {.kind = EOF_TOKEN_}; break;
        case '\n': return (Token) {.kind = EOF_TOKEN_}; break;
        case '(':  return (Token) {.kind = LPAREN,  .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case ')':  return (Token) {.kind = RPAREN,  .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case '+':  return (Token) {.kind = PLUS,    .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case '-':  return (Token) {.kind = MINUS,   .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case '*':  return (Token) {.kind = STAR,    .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case '/':  return (Token) {.kind = FSLASH,  .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case '%':  return (Token) {.kind = PERCENT, .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case ',':  return (Token) {.kind = COMMA,   .text = hgl_sv_substr(l->buf, 0, 1)}; break;

        case 'a' ... 'z': case 'A' ... 'Z': case '_': {
            size_t i = 1;
            for (; i < l->buf.length; i++) {
                c = l->buf.start[i];
                if (!is_identifier_char(c)) break;
            }

            return (Token) {
                .kind = IDENTIFIER, 
                .text = hgl_sv_substr(l->buf, 0, i),
            };
        } break;

        case '0' ... '9': {
            size_t i = 1;
            for (; i < l->buf.length; i++) {
                c = l->buf.start[i];
                if (!is_decimal_char(c)) break;
            }

            /* int literal */
            if (c != '.') {
                if (i != l->buf.length) LEXER_ASSERT(!is_identifier_char(c));
                return (Token) {
                    .kind = INT_LITERAL,
                    .text = hgl_sv_substr(l->buf, 0, i),
                };
            }

            /* float literal */
            i++;
            for (; i < l->buf.length; i++) {
                c = l->buf.start[i];
                if (!is_decimal_char(c)) break;
            }

            if (i != l->buf.length) LEXER_ASSERT(!is_identifier_char(c));
            return (Token) {
                .kind = FLOAT_LITERAL,
                .text = hgl_sv_substr(l->buf, 0, i),
            };

        } break;
    }

    LEXER_ASSERT(!is_identifier_char(c));
    return (Token) {.kind = EOF_TOKEN_};
}

//void token_print(Token *token)
//{
//    printf("Token [%d]: " HGL_SV_FMT "\n", token->kind, HGL_SV_ARG(token->text));
//}

void token_print(Token *token)
{
    printf(HGL_SV_FMT, HGL_SV_ARG(token->text));
}

/*--- Private functions -----------------------------------------------------------------*/

//static bool is_identifier_start_char(int c)
//{
//    return (c >= 'a' && c <= 'z') ||
//           (c >= 'A' && c <= 'Z') ||
//           (c == '_');
//}

static bool is_identifier_char(int c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           (c == '_');
}

//static bool is_hex_char(int c)
//{
//    return (c >= 'a' && c <= 'f') ||
//           (c >= 'A' && c <= 'F') ||
//           (c >= '0' && c <= '9') ||
//           (c == '_');
//}
//
//static bool is_octal_char(int c)
//{
//    return (c >= '0' && c <= '7') ||
//           (c == '_');
//}

static bool is_decimal_char(int c)
{
    return (c >= '0' && c <= '9') ||
           (c == '_');
}

