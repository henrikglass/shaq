#ifndef LOG_H
#define LOG_H

/*--- Include files ---------------------------------------------------------------------*/

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
const char *log_get_info_log(void);
const char *log_get_error_log(void);

#endif /* LOG_H */

