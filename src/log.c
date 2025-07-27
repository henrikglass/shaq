/*--- Include files ---------------------------------------------------------------------*/

#include "log.h"
#include "array.h"
#include "hgl_int.h"

#include <stdio.h>

/*--- Private macros --------------------------------------------------------------------*/

#define ANSI_RED       "\033[31m"
#define ANSI_GREEN     "\033[32m"
#define ANSI_NC        "\033[0m"

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct {
    Array(char, 128*1024) info_buffer;
    Array(u32,  128)      info_entries;
    u32 info_iterator;

    Array(char, 128*1024) error_buffer;
    Array(u32,  128)      error_entries;
    u32 error_iterator;
} logs;

/*--- Public functions ------------------------------------------------------------------*/

void log_clear_all_logs()
{
    array_clear(&logs.info_buffer);
    array_clear(&logs.info_entries);
    logs.info_iterator = 0;

    array_clear(&logs.error_buffer);
    array_clear(&logs.error_entries);
    logs.error_iterator = 0;
}

void log_info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    array_push(&logs.info_entries, logs.info_buffer.count);
    logs.info_buffer.count += vsprintf(&logs.info_buffer.arr[logs.info_buffer.count], fmt, args);
    logs.info_buffer.arr[logs.info_buffer.count++] = '\0';
    va_end(args);
}

void log_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    array_push(&logs.error_entries, logs.error_buffer.count);
    logs.error_buffer.count += vsprintf(&logs.error_buffer.arr[logs.error_buffer.count], fmt, args);
    logs.error_buffer.arr[logs.error_buffer.count++] = '\0';
    va_end(args);
}

void log_print_info_log()
{
    //fprintf(stdout, "%.*s", info_buffer.count, info_buffer.arr);
    while (true) {
        u32 msg_len;
        const char *msg = log_get_next_info_msg(&msg_len);
        if (msg == NULL) {
            break;
        }
        fprintf(stdout, "[" ANSI_GREEN "INFO" ANSI_NC "] ");
        fprintf(stdout, "%.*s\n", msg_len, msg);
    }
    fflush(stdout);
}

void log_print_error_log()
{
    //fprintf(stderr, "%.*s", error_buffer.count, error_buffer.arr);
    while (true) {
        u32 msg_len;
        const char *msg = log_get_next_error_msg(&msg_len);
        if (msg == NULL) {
            break;
        }
        fprintf(stderr, "[" ANSI_RED "ERROR" ANSI_NC "] ");
        fprintf(stderr, "%.*s\n", msg_len, msg);
    }
    fflush(stderr);
}

//const char *log_get_info_log(u32 *length)
//{
//    *length = logs.info_buffer.count;
//    return logs.info_buffer.arr;
//}
//
//const char *log_get_error_log(u32 *length)
//{
//    *length = logs.error_buffer.count;
//    return logs.error_buffer.arr;
//}

const char *log_get_next_info_msg(u32 *length)
{
    if (logs.info_iterator == logs.info_entries.count) {
        logs.info_iterator = 0;
        *length = 0;
        return NULL;
    }

    b8 last = (logs.info_iterator == (logs.info_entries.count - 1));
    u32 offset = logs.info_entries.arr[logs.info_iterator];
    u32 next_offset = last ? logs.info_buffer.count :
                             logs.info_entries.arr[logs.info_iterator + 1];
    *length = next_offset - offset - 1;
    logs.info_iterator++;
    return &logs.info_buffer.arr[offset];
}

const char *log_get_next_error_msg(u32 *length)
{
    if (logs.error_iterator == logs.error_entries.count) {
        logs.error_iterator = 0;
        *length = 0;
        return NULL;
    }

    b8 last = (logs.error_iterator == (logs.error_entries.count - 1));
    u32 offset = logs.error_entries.arr[logs.error_iterator];
    u32 next_offset = last ? logs.error_buffer.count :
                             logs.error_entries.arr[logs.error_iterator + 1];
    *length = next_offset - offset - 1;
    logs.error_iterator++;
    return &logs.error_buffer.arr[offset];
}




/*--- Private functions -----------------------------------------------------------------*/

