/*--- Include files ---------------------------------------------------------------------*/

#include "util.h"

#include <time.h>
#include <stdio.h>
#include <sys/stat.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

u64 util_get_time_nanos(void)
{
    struct timespec t_temp;
    clock_gettime(CLOCK_MONOTONIC, &t_temp);
    u64 ns = t_temp.tv_sec * 1000000000ll + t_temp.tv_nsec;
    return ns;
}

i64 util_get_file_modify_time(const char *filepath)
{
    struct stat statbuf;
    int err = stat(filepath, &statbuf);
    if (err != 0) {
        fprintf(stderr, "error trying to get modify-time for `%s`\n", filepath);
        return (time_t)-1;
    }
    return (i64) statbuf.st_mtime;
}

/*--- Private functions -----------------------------------------------------------------*/

