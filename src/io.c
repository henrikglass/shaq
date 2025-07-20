/*--- Include files ---------------------------------------------------------------------*/

#include "io.h"
#include "alloc.h"

#include "hgl_int.h"

#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

i64 io_get_file_modify_time(const char *filepath)
{
    struct stat statbuf;
    int err = stat(filepath, &statbuf);
    if (err != 0) {
        fprintf(stderr, "error trying to get modify-time for `%s`. Errno = %s\n", 
                filepath, strerror(errno));
        return -1;
    }
    return (i64) statbuf.st_mtime;
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
    u8 *file_data = fs_alloc(g_longterm_fs_allocator, file_size);
    if (file_data == NULL) {
        goto out;
    }

    /* read file */
    ssize_t n_read_bytes = fread(file_data, 1, file_size, fp);
    if (n_read_bytes != file_size) {
        fs_free(g_longterm_fs_allocator, file_data);
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

