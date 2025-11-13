#ifndef LOG_H
#define LOG_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"

#include <stdarg.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void log_info(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_clear_all_logs(void);
void log_print_info_log(void);
void log_print_error_log(void);
b8 log_info_log_is_empty(void);
b8 log_error_log_is_empty(void);
const char *log_get_next_info_msg(u32 *length);
const char *log_get_next_error_msg(u32 *length);

#endif /* LOG_H */

