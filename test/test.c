#include "hgl_test.h"

#include "sel.h"
#include "alloc.h"
#include "log.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

/* Expose internal sel.c functions */
ExprTree *parse_expr(const char *str);
TypeAndQualifier type_and_namecheck(ExprTree *e);

GLOBAL_SETUP {
    log_set_mode(LOG_R2R);
    alloc_init();
}

ON_ASSERT_PASS {
    log_clear();
}

ON_ASSERT_FAIL {
    log_print();
}

#define type_of(str)      type_and_namecheck(parse_expr(str)).type
#define qualifier_of(str) type_and_namecheck(parse_expr((str))).qualifier
#define run(str)                                               \
    ({                                                         \
        ExeExpr *exe = sel_compile((str), g_r2r_fs_allocator); \
        sel_eval(exe, (SVMContext){0}, true);                  \
    })

TEST(test_parse_simple_expressions)
{
    /* these should fail */
    ASSERT(NULL == parse_expr(""));
    ASSERT(NULL == parse_expr("1+"));
    ASSERT(NULL == parse_expr(","));
    ASSERT(NULL == parse_expr("123*"));
    ASSERT(NULL == parse_expr("*"));
    ASSERT(NULL == parse_expr("23/"));
    ASSERT(NULL == parse_expr("1.xx"));
    ASSERT(NULL == parse_expr("func(,)"));
    ASSERT(NULL == parse_expr("func(23,,23)"));
    //ASSERT(NULL == parse_expr("func(23,)")); // TODO FIX
    //ASSERT(NULL == parse_expr("func(1, 2, 3, asdf,)")); // TODO FIX

    /* these should succeed */
    ASSERT(NULL != parse_expr("1"));
    ASSERT(NULL != parse_expr("1123"));
    ASSERT(NULL != parse_expr("1123+123"));
    ASSERT(NULL != parse_expr("1123+123.123"));
    ASSERT(NULL != parse_expr("4+2.3"));
    ASSERT(NULL != parse_expr("(sin(time()))*23.0"));
    ASSERT(NULL != parse_expr("1.0.xx"));
    ASSERT(NULL != parse_expr("(1).xx"));
    ASSERT(NULL != parse_expr("(1).xyzw"));
    ASSERT(NULL != parse_expr("(1).mopc"));
    ASSERT(NULL != parse_expr("func(1, 2, 3, asdf, \"kalas\")"));
}

TEST(test_namecheck_simple_expressions)
{
    /* these should fail (return TYPE_OR_NAME_ERR_) because of name errors */
    ASSERT(TYPE_OR_NAME_ERR_ == type_of("this_does_not_exist"));
    ASSERT(TYPE_OR_NAME_ERR_ == type_of("this_does_not_exist()"));
    ASSERT(TYPE_OR_NAME_ERR_ == type_of("sin(PI_FROM_ANOTHER_UNIVERSE)"));

    /* these should pass because the names exist */
    ASSERT(TYPE_FLOAT == type_of("PI"));
}

TEST(test_type_and_namecheck_simple_expressions)
{
    /* these should fail (return TYPE_OR_NAME_ERR_) because of name errors */
    ASSERT(TYPE_OR_NAME_ERR_ == type_of("this_does_not_exist"));
    ASSERT(TYPE_OR_NAME_ERR_ == type_of("this_does_not_exist()"));
    ASSERT(TYPE_OR_NAME_ERR_ == type_of("sin(PI_FROM_ANOTHER_UNIVERSE)"));

    /* these should pass because the names exist */
    ASSERT(TYPE_FLOAT == type_of("PI"));

    /* These should all pass */
    ASSERT(TYPE_INT == type_of("1"));
    ASSERT(TYPE_INT == type_of("191912"));
    ASSERT(TYPE_INT == type_of("0x1234"));
    ASSERT(TYPE_INT == type_of("01234"));
    ASSERT(TYPE_INT == type_of("int(1.23)"));
    ASSERT(TYPE_INT == type_of("int(32u)"));
    ASSERT(TYPE_UINT == type_of("1u"));
    ASSERT(TYPE_UINT == type_of("191912u"));
    ASSERT(TYPE_UINT == type_of("0x1234u"));
    ASSERT(TYPE_UINT == type_of("01234u"));
    ASSERT(TYPE_UINT == type_of("unsigned(0x1234)"));
    ASSERT(TYPE_UINT == type_of("unsigned(01234)"));
    ASSERT(TYPE_FLOAT == type_of("23+10.0"));
    ASSERT(TYPE_FLOAT == type_of("1."));
    ASSERT(TYPE_FLOAT == type_of("123.456"));
    ASSERT(TYPE_FLOAT == type_of("sin(PI)"));
    ASSERT(TYPE_VEC2 == type_of("1.0.xx"));
    ASSERT(TYPE_VEC2 == type_of("(1.0).xx"));
    ASSERT(TYPE_IVEC2 == type_of("(1).xx"));
    ASSERT(TYPE_IVEC2 == type_of("((1).xx)"));
    ASSERT(TYPE_IVEC4 == type_of("((1).xx).xyxy"));
    ASSERT(TYPE_IVEC4 == type_of("((1).xx).uvuv"));
    ASSERT(TYPE_IVEC4 == type_of("((1).xx).sstt"));
    ASSERT(TYPE_STR == type_of("\"\""));
    ASSERT(TYPE_STR == type_of("\"hejsan\""));
    ASSERT(TYPE_VEC3 == type_of("mat3_id()*vec3(1.0, 2.0, float(3))"));

    /* These swizzles should fail */
    ASSERT(TYPE_OR_NAME_ERR_ == type_of("(1).xyzw"));
    ASSERT(TYPE_OR_NAME_ERR_ == type_of("(1).xyxy"));
    ASSERT(TYPE_OR_NAME_ERR_ == type_of("((1).xxxx).mopc"));
    ASSERT(TYPE_OR_NAME_ERR_ == type_of("((1).xxxx).xyzxyz"));

    /* These math ops should fail */
    ASSERT(TYPE_OR_NAME_ERR_ == type_of("vec3(1.0, 2.0, float(3))*mat3_id()"));

}

TEST(test_stdin_nonblock, .timeout = 1.0f)
{
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
    run("stdin()");
}

TEST(test_qualifiers)
{
    ASSERT(QUALIFIER_CONST == qualifier_of("191912"));
    ASSERT(QUALIFIER_CONST == qualifier_of("int(1.23)"));
    ASSERT(QUALIFIER_CONST == qualifier_of("unsigned(01234)"));
    ASSERT(QUALIFIER_CONST == qualifier_of("sin(PI)"));
    ASSERT(QUALIFIER_NONE == qualifier_of("sin(time())"));
    ASSERT(QUALIFIER_NONE == qualifier_of("stdin()"));
    ASSERT(QUALIFIER_NONE == qualifier_of("sin(PI) + time()"));
    ASSERT(QUALIFIER_NONE == qualifier_of("sin(PI) * time()"));
    ASSERT(QUALIFIER_NONE == qualifier_of("sin(PI) - time()"));
    ASSERT(QUALIFIER_NONE == qualifier_of("sin(PI) / time()"));
}

