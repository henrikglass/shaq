#ifndef UNIFORM_H
#define UNIFORM_H

/*--- Include files ---------------------------------------------------------------------*/

#include "sel.h"
#include "str.h"

#include "hgl_ini.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct {
    StringView name;
    Type type;
    ExeExpr *exe;
} Uniform;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

i32 uniform_parse_from_ini_kv_pair(Uniform *u, HglIniKVPair *kv);

#endif /* UNIFORM_H */

