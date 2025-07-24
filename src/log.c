/*--- Include files ---------------------------------------------------------------------*/

#include "log.h"
#include "array.h"
#include "hgl_int.h"

#include <stdio.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

Array(char, 128*1024) info_log;
Array(char, 128*1024) error_log;

/*--- Public functions ------------------------------------------------------------------*/

void log_clear_all_logs()
{
    array_clear(&info_log);
    array_clear(&error_log);
    info_log.arr[0] = '\0';
    error_log.arr[0] = '\0';
}

void log_info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    info_log.count += sprintf(&info_log.arr[info_log.count], "[INFO] ");
    info_log.count += vsprintf(&info_log.arr[info_log.count], fmt, args);
    info_log.count += sprintf(&info_log.arr[info_log.count], "\n"); 
    info_log.arr[info_log.count] = '\0';
    va_end(args);
}

void log_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    error_log.count += sprintf(&error_log.arr[error_log.count], "[ERROR] ");
    error_log.count += vsprintf(&error_log.arr[error_log.count], fmt, args);
    error_log.count += sprintf(&error_log.arr[error_log.count], "\n"); 
    error_log.arr[error_log.count] = '\0';
    va_end(args);
}

void log_print_info_log()
{
    fprintf(stdout, "%s", info_log.arr);
    fflush(stdout);
}

void log_print_error_log()
{
    fprintf(stderr, "%s", error_log.arr);
    fflush(stderr);
}

const char *log_get_info_log()
{
    return info_log.arr;
}

const char *log_get_error_log()
{
    return error_log.arr;
}



/*--- Private functions -----------------------------------------------------------------*/

