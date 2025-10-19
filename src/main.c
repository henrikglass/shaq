#include <stdio.h>

#include "sel.h"
#include "shaq_core.h"

#define HGL_FLAGS_IMPLEMENTATION
#include "hgl_flags.h"

#include <time.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    const char **opt_input = hgl_flags_add_str("-i,--input", "The input project (.ini) file to run", NULL, 0);
    u64 *opt_rng_seed = hgl_flags_add_u64("-s,--seed", "`srand()` seed (defaults to `time(NULL)`)", 0, 0);
    bool *opt_list_builtins = hgl_flags_add_bool("-l,--list-builtins", "List the built-in functions and constants in the Simple Expression Language (SEL)", false, 0);
    bool *opt_quiet = hgl_flags_add_bool("-q,--quiet", "Less verbose log messages on stdout/stderr", false, 0);
    bool *opt_help = hgl_flags_add_bool("-help,--help", "Display this message", false, 0);

    i32 err = hgl_flags_parse(argc, argv);
    if (err != 0 || *opt_help) {
        printf("Usage: %s [Options]\n", argv[0]);
        hgl_flags_print();
        return (err != 0) ? 1 : 0;
    }

    if (*opt_list_builtins) {
        sel_list_builtins();
        return 0;
    }
   
    if (*opt_input == NULL) {
        fprintf(stderr, "No input file (*.ini) provided.\n");
        printf("Usage: %s [Options]\n", argv[0]);
        hgl_flags_print();
        return 1;
    }

    srand(*opt_rng_seed == 0 ? (u64)time(NULL): *opt_rng_seed);

    //shaq_begin(*opt_input, *opt_quiet);
    shaq_begin(NULL, *opt_quiet);
    while (!shaq_should_close()) {
        shaq_new_frame();
    }
    shaq_end();
}

// TODO SEL: Better error handling & error messages
// TODO Remove alloc.h/.c once the program structure is more coherent
