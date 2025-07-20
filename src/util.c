/*--- Include files ---------------------------------------------------------------------*/

#include "util.h"

#include <time.h>
#include <stdio.h>

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

/*--- Private functions -----------------------------------------------------------------*/

