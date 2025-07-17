#ifndef UTIL_H
#define UTIL_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

u64 util_get_time_nanos(void);
i64 util_get_file_modify_time(const char *filepath);

#endif /* UTIL_H */

