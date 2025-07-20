#ifndef IO_H
#define IO_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"

#include <stddef.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

i64 io_get_file_modify_time(const char *filepath);
u8 *io_read_entire_file(const char *filepath, size_t *size);

#endif /* IO_H */

