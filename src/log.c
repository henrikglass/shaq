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

typedef struct
{
    const char *cstr;
    u32 length;
    LogEntryKind kind;
} LogEntry;

typedef struct
{
    Array(char, 256*1024) buffer;
    Array(LogEntry,  256) entries;
    b8 has_info;
    b8 has_error;
} Log;

/*--- Private function prototypes -------------------------------------------------------*/

static inline LogEntry *next_entry(void);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/


static struct 
{
    LogMode mode;
    Log r2r_log;
    Log frame_log;
    u32 it;
} logs;

/*--- Public functions ------------------------------------------------------------------*/

void log_set_mode(LogMode mode)
{
    logs.mode = mode;
}

LogMode log_get_mode()
{
    return logs.mode;
}

void log_info(const char *fmt, ...)
{
    Log *l = NULL;
    switch (logs.mode) {
        case LOG_R2R:   l = &logs.r2r_log; break;
        case LOG_FRAME: l = &logs.frame_log; break;
        case LOG_DISABLE: return;
    }
    l->has_info = true;

    va_list args;
    va_start(args, fmt);
    u32 offset = l->buffer.count;
    char *cstr = &l->buffer.arr[offset];
    l->buffer.count += vsprintf(cstr, fmt, args);
    l->buffer.arr[l->buffer.count++] = '\0';
    u32 length = l->buffer.count - offset - 1;
    LogEntry e = {
        .cstr   = cstr,
        .length = length,
        .kind   = LOG_INFO,
    };
    array_push(&l->entries, e);
    va_end(args);
}

void log_error(const char *fmt, ...)
{
    Log *l = NULL;
    switch (logs.mode) {
        case LOG_R2R:   l = &logs.r2r_log; break;
        case LOG_FRAME: l = &logs.frame_log; break;
        case LOG_DISABLE: return;
    }
    l->has_error = true;

    va_list args;
    va_start(args, fmt);
    u32 offset = l->buffer.count;
    char *cstr = &l->buffer.arr[offset];
    l->buffer.count += vsprintf(cstr, fmt, args);
    l->buffer.arr[l->buffer.count++] = '\0';
    u32 length = l->buffer.count - offset - 1;
    LogEntry e = {
        .cstr   = cstr,
        .length = length,
        .kind   = LOG_ERROR,
    };
    array_push(&l->entries, e);
    va_end(args);
}

void log_clear()
{
    if (logs.mode == LOG_R2R) {
        array_clear(&logs.r2r_log.buffer);
        array_clear(&logs.r2r_log.entries);
        logs.r2r_log.has_info = false;
        logs.r2r_log.has_error = false;
    }

    array_clear(&logs.frame_log.buffer);
    array_clear(&logs.frame_log.entries);
    logs.frame_log.has_info = false;
    logs.frame_log.has_error = false;
}

void log_print()
{
    while (true) {
        LogEntry *e = next_entry();
        if (e == NULL) {
            break;
        }
        switch (e->kind) {
            case LOG_INFO: {
                fprintf(stdout, "[" ANSI_GREEN "INFO" ANSI_NC "] ");
                fprintf(stdout, "%.*s\n", e->length, e->cstr);
            } break;

            case LOG_ERROR: {
                fprintf(stderr, "[" ANSI_RED "ERROR" ANSI_NC "] ");
                fprintf(stderr, "%.*s\n", e->length, e->cstr);
            } break;
        }
    }
    fflush(stdout);
    fflush(stderr);
}

//void log_print_frame()
//{
//    for (u32 i = 0; i < logs.frame_log.entries.count; i++) {
//        LogEntry *e = &logs.frame_log.entries.arr[i];
//        if (e == NULL) {
//            break;
//        }
//        switch (e->kind) {
//            case LOG_INFO: {
//                fprintf(stdout, "[" ANSI_GREEN "INFO" ANSI_NC "] ");
//                fprintf(stdout, "%.*s\n", e->length, e->cstr);
//            } break;
//
//            case LOG_ERROR: {
//                fprintf(stderr, "[" ANSI_RED "ERROR" ANSI_NC "] ");
//                fprintf(stderr, "%.*s\n", e->length, e->cstr);
//            } break;
//        }
//    }
//    fflush(stdout);
//    fflush(stderr);
//}

b8 log_has_info()
{
    return logs.r2r_log.has_info ||
           logs.frame_log.has_info;
}

b8 log_has_error()
{
    return logs.r2r_log.has_error ||
           logs.frame_log.has_error;
}

const char *log_get_next_msg(u32 *length, LogEntryKind *kind)
{
    LogEntry *e = next_entry();
    if (e == NULL) {
        return NULL;
    }
    if (length != NULL) {
        *length = e->length;
    }
    if (kind != NULL) {
        *kind = e->kind;
    }
    return e->cstr;
}

const char *log_get_next_info_msg(u32 *length)
{
    while (true) {
        LogEntry *e = next_entry();
        if (e == NULL) {
            return NULL;
        }
        if (e->kind == LOG_ERROR) {
            continue;
        }
        if (length != NULL) {
            *length = e->length;
        }
        return e->cstr;
    }
}

const char *log_get_next_error_msg(u32 *length)
{
    while (true) {
        LogEntry *e = next_entry();
        if (e == NULL) {
            return NULL;
        }
        if (e->kind == LOG_INFO) {
            continue;
        }
        if (length != NULL) {
            *length = e->length;
        }
        return e->cstr;
    }
}

/*--- Private functions -----------------------------------------------------------------*/

static inline LogEntry *next_entry()
{
    u32 n_total_entries = logs.r2r_log.entries.count +
                          logs.frame_log.entries.count;
    if (logs.it == n_total_entries) {
        logs.it = 0;
        return NULL;
    }

    LogEntry *e;
    if (logs.it < logs.r2r_log.entries.count) {
        u32 idx = logs.it;
        e = &logs.r2r_log.entries.arr[idx];
    } else {
        u32 idx = logs.it - logs.r2r_log.entries.count;
        e = &logs.frame_log.entries.arr[idx];
    }

    logs.it++;
    return e;
}

