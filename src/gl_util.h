#ifndef GL_UTIL_H
#define GL_UTIL_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

#define gl_check_errors() gl_check_errors_(__FILE__, __LINE__)
i32 gl_check_errors_(const char *file, i32 line);

#endif /* GL_UTIL_H */

