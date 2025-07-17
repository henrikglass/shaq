#ifndef UNIFORM_H
#define UNIFORM_H

/*--- Include files ---------------------------------------------------------------------*/

#include "sel.h"

#include "hgl_string.h"
#include "hgl_ini.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct {
    HglStringView name;
    Type type;
    ExeExpr *exe;
} Uniform;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

int uniform_parse_from_ini_kv_pair(Uniform *u, HglIniKVPair *kv);

#endif /* UNIFORM_H */

