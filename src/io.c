/*--- Include files ---------------------------------------------------------------------*/

#include "io.h"
#include "alloc.h"

#include "hgl_int.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

i64 io_get_file_modify_time(const char *filepath, bool retry_on_failure)
{
    i64 ret = -1;
    if (filepath == NULL) {
        goto out; 
    }

    /*
     * What the fuck is this, you may ask?
     *
     * Explanation: `open()` will fail with errno = "No such file or directory" 
     * very briefly after the file at `filepath` has been modified (at least by 
     * vim).
     *
     * Workaround: Retry a couple times during a short period (< 1s), otherwise
     * fail.
     *
     * TODO: Figure out a better solution.
     */
    i32 fd = open(filepath, O_RDONLY);
    if (retry_on_failure) {
        i32 n_retries_left = 10;
        while (fd == -1 && n_retries_left > 0) {
            struct timespec ts = {.tv_sec = 0, .tv_nsec = 16666667};
            nanosleep(&ts, &ts);
            fd = open(filepath, O_RDONLY);
            n_retries_left--;
        }
    }
    if (fd == -1) {
        goto out;
    }

    struct stat statbuf;
    i32 err = fstat(fd, &statbuf);
    if (err == -1) {
        goto out;
    }
    
    ret = (i64) statbuf.st_mtime;
out:
    if (fd != -1) {
        close(fd);
    }
    return ret;
}

u8 *io_read_entire_file(const char *filepath, size_t *size)
{
    u8 *data = NULL;
    *size = 0;

    /* open file in read binary mode */
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL) {
        goto out;
    }

    /* get file size */
    fseek(fp, 0, SEEK_END);
    ssize_t file_size = ftell(fp);
    rewind(fp);
    if (file_size < 0) {
        goto out;
    }

    /* allocate memory for data */
    u8 *file_data = hgl_alloc(g_session_fs_allocator, file_size);
    if (file_data == NULL) {
        goto out;
    }

    /* read file */
    ssize_t n_read_bytes = fread(file_data, 1, file_size, fp);
    if (n_read_bytes != file_size) {
        hgl_free(g_session_fs_allocator, file_data);
        goto out;
    }

    /* populate `data` & `size` before returning */
    data = file_data;
    *size = (size_t) file_size;

out:
    if (fp != NULL) {
        fclose(fp);
    }
    return data;
}

/*--- Private functions -----------------------------------------------------------------*/

