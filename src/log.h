#ifndef LOG_H
#define LOG_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"

#include <stdarg.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    LOG_INFO,
    LOG_ERROR,
} LogEntryKind;

typedef enum
{
    LOG_R2R,
    LOG_FRAME,
    LOG_DISABLE,
} LogMode;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void log_set_mode(LogMode mode);
LogMode log_get_mode(void);
void log_info(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_clear(void);
void log_print(void);
//void log_print_frame(void); // TODO neater API pls
b8 log_has_info(void);
b8 log_has_error(void);
const char *log_get_next_msg(u32 *length, LogEntryKind *kind);
const char *log_get_next_info_msg(u32 *length);
const char *log_get_next_error_msg(u32 *length);

#endif /* LOG_H */

