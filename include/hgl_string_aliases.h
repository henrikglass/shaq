

/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2025 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_string.h implements a dynamic string builder and string view.
 *
 *
 * USAGE:
 *
 * Include hgl_string.h file like this:
 *
 *     #define HGL_STRING_IMPLEMENTATION
 *     #include "hgl_memdbg.h"
 *
 * HGL_STRING_IMPLEMENTATION must only be defined once, in a single compilation unit.
 *
 * hgl_string.h implements two types: HglStringBuilder and HglStringView. HglStringBuilder
 * is a mutable null-terminated string type. HglStringView is an immutable optionally
 * null-terminated string type (although the "immutable" part should be taking with a
 * grain of salt - this is still C).
 *
 * hgl_string.h allows the default allocator, reallocator and free function to be
 * overridden by redefining the following defines before including hgl_string.h:
 *
 *     #define HGL_STRING_ALLOC    malloc
 *     #define HGL_STRING_REALLOC  realloc
 *     #define HGL_STRING_FREE     free
 *
 * Allocator functions may also be overridden per-object in the case of HglStringBuilder
 * by passing them as arguments in the hgl_sb_make() macro, as such:
 *
 *     HglStringBuilder sb = hgl_sb_make(.alloc   = my_alloc,
 *                                       .realloc = my_realloc,
 *                                       .free    = my_free);
 *
 * Allocators not specified in the arguments to `hgl_sb_make` inhertit from the respective
 * HGL_STRING_ALLOC, HGL_STRING_REALLOC, and HGL_STRING_FREE macros. It's is completely
 * legal to specify only one or two of these functions, but some operations may break.
 *
 * EXAMPLE:
 *
 * In this example, we create a string builder, append the contents of a file to the
 * string builder, split the contents by line, and print out the number of occurances
 * of the word "glass" on each line:
 *
 *     HglStringBuilder sb = hgl_sb_make(.initial_capacity = 4096); // alt. only `hgl_sb_make()` is enough.
 *     hgl_sb_append_file(&sb, "assets/glassigt_lyrics.txt");
 *     HglStringView sv = hgl_sv_from_sb(&sb);
 *
 *     int line_nr = 1;
 *     hgl_sv_op_begin(&sv);
 *     HglStringView line = hgl_sv_split_next(&sv, '\n');
 *     while (line.start != NULL) {
 *         int occurances = 0;
 *         hgl_sv_op_begin(&line);
 *         while (hgl_sv_find_next(&line, "glassigt").start != NULL) {
 *             occurances++;
 *         }
 *         printf("line #%d has %d occurances of glassigt.\n", line_nr++, occurances);
 *         line = hgl_sv_split_next(&sv, '\n');
 *     }
 *
 *     hgl_sb_destroy(&sb);
 *
 * See examples/test_string.c for more examples.
 *
 *
 * TODO:
 *     * Don't call hgl_sb_grow_by_policy when not necessary. Check length before.
 *     * sb prepend? Or maybe not?
 *     * sb lchop & rchop? Or maybe not?
 *
 * AUTHOR: Henrik A. Glass
 *
 */

typedef HglStringView StringView;
typedef HglStringBuilder StringBuilder;

#define SV_LIT HGL_SV_LIT
#define SV_FMT HGL_SV_FMT
#define SV_ARG HGL_SV_ARG
#define SB_FMT HGL_SB_FMT
#define SB_ARG HGL_SB_ARG

#define sv_from                  hgl_sv_from
#define sv_from_sb               hgl_sv_from_sb
#define sv_from_cstr             hgl_sv_from_cstr
#define sv_make_cstr_copy        hgl_sv_make_cstr_copy
#define sv_op_begin              hgl_sv_op_begin
#define sv_split_next            hgl_sv_split_next
#define sv_find_next             hgl_sv_find_next
#define sv_find_next_regex_match hgl_sv_find_next_regex_match
#define sv_substr                hgl_sv_substr
#define sv_lchop                 hgl_sv_lchop
#define sv_rchop                 hgl_sv_rchop
#define sv_lchop_until           hgl_sv_lchop_until
#define sv_rchop_until           hgl_sv_rchop_until
#define sv_lchop_lexeme          hgl_sv_lchop_lexeme
#define sv_ltrim                 hgl_sv_ltrim
#define sv_rtrim                 hgl_sv_rtrim
#define sv_trim                  hgl_sv_trim
#define sv_to_u64                hgl_sv_to_u64
#define sv_to_i64                hgl_sv_to_i64
#define sv_to_f64                hgl_sv_to_f64
#define sv_lchop_u64             hgl_sv_lchop_u64
#define sv_lchop_i64             hgl_sv_lchop_i64
#define sv_lchop_f64             hgl_sv_lchop_f64
#define sv_starts_with_lchop     hgl_sv_starts_with_lchop
#define sv_starts_with_lexeme    hgl_sv_starts_with_lexeme
#define sv_contains              hgl_sv_contains
#define sv_starts_with           hgl_sv_starts_with
#define sv_ends_with             hgl_sv_ends_with
#define sv_compare               hgl_sv_compare
#define sv_equals                hgl_sv_equals

#define sb_make                  hgl_sb_make
#define sb_make_copy             hgl_sb_make_copy
#define sb_destroy               hgl_sb_destroy
#define sb_clear                 hgl_sb_clear
#define sb_grow                  hgl_sb_grow
#define sb_grow_by_policy        hgl_sb_grow_by_policy
#define sb_shrink_to_fit         hgl_sb_shrink_to_fit
#define sb_append                hgl_sb_append
#define sb_append_char           hgl_sb_append_char
#define sb_append_sv             hgl_sb_append_sv
#define sb_append_cstr           hgl_sb_append_cstr
#define sb_append_fmt            hgl_sb_append_fmt
#define sb_append_file           hgl_sb_append_file
#define sb_replace_section       hgl_sb_replace_section
#define sb_replace               hgl_sb_replace
#define sb_replace_regex         hgl_sb_replace_regex
#define sb_rtrim                 hgl_sb_rtrim
#define sb_ltrim                 hgl_sb_ltrim
#define sb_trim                  hgl_sb_trim
#define sb_rchop                 hgl_sb_rchop

