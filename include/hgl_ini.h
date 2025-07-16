
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2023 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_ini.h implements a simple *.ini file parser.
 *
 *
 * USAGE:
 *
 * Include hgl_ini.h file like this:
 *
 *     #define HGL_INI_IMPLEMENTATION
 *     #include "hgl_ini.h"
 *
 * Code example:
 *
 *     HglIni *ini = hgl_ini_open("my_ini_file.ini");
 *     if (ini == NULL) {
 *         fprintf(stderr, "Uh oh parsing failed for some reason.\n");
 *         return 1;
 *     }
 *
 *     // read
 *     const char *my_text = hgl_ini_get(ini, "MySection", "MyKeyName");
 *     bool my_bool        = hgl_ini_get_bool(ini, "MyOtherSection", "MyBool");
 *     uint64_t my_u64     = hgl_ini_get_u64(ini, "MyNumbers", "MyUnsignedNumber");
 *     int64_t my_i64      = hgl_ini_get_i64(ini, "MyNumbers", "MySignedNumber");
 *     float my_f32        = (float) hgl_ini_get_f64(ini, "MyNumbers", "MyFloat");
 *
 *     // write
 *     hgl_ini_put(ini, "MyNewSection", "MyNewKey", "MyNewValue");
 *
 *     // Save to "my_ini_file.ini".
 *     hgl_ini_save(ini, NULL);
 *     hgl_ini_close(ini);
 *
 * See examples/test_ini.c for more usage examples.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */


#ifndef HGL_INI_H
#define HGL_INI_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#define HglIniDa(T)        \
    struct {               \
        T *arr;            \
        size_t count;      \
        size_t capacity;   \
        size_t iterator;   \
    }

typedef HglIniDa(struct HglIniKVPair) HglIniKVPairs;
typedef HglIniDa(struct HglIniSection) HglIniSections;

typedef struct HglIniKVPair
{
    char *key;
    char *val;
} HglIniKVPair;

typedef struct HglIniSection
{
    char *name;
    HglIniKVPairs kv_pairs;
} HglIniSection;

typedef struct
{
    char *filepath;
    HglIniSections sections;
} HglIni;

/**
 * Creates a blank HglIni object.
 */
HglIni *hgl_ini_create(const char *filepath);

/**
 * Parses the *.ini file at `filepath`.
 */
HglIni *hgl_ini_open(const char *filepath);

/**
 * Saves `ìni` as a file at `filepath`. If `filepath` is NULL, the file
 * will be saved to `ini->filepath`.
 */
void hgl_ini_save(HglIni *ini, const char *filepath);

/**
 * Frees the memory used by `ini`.
 */
void hgl_ini_close(HglIni *ini);

/**
 * Returns a pointer to the next section in `ini` or NULL if there
 * are no more sections left.
 *
 * To restart the iteration (i.e. such that the next call to
 * `hgl_ini_next_section` returns the first section), you must call
 * `hgl_ini_reset_section_iterator()` with `ini`.
 */
HglIniSection *hgl_ini_next_section(HglIni *ini);

/**
 * Returns a pointer to the next key-value pair in `section` or NULL
 * if there are no more key-value pairs left.
 *
 * To restart the iteration (i.e. such that the next call to
 * `hgl_ini_next_kv_pair` returns the first key-value pair), you
 * must call `hgl_ini_reset_kv_pair_iterator()` with `section`.
 */
HglIniKVPair *hgl_ini_next_kv_pair(HglIniSection *section);

/**
 * Reset's the internal iterator of `ini` such that the next call to
 * hgl_ini_next_section() returns the first section.
 */
void hgl_ini_reset_section_iterator(HglIni *ini);

/**
 * Reset's the internal iterator of `section` such that the next call to
 * hgl_ini_next_kv_pair() returns the first key-value-pair.
 */
void hgl_ini_reset_kv_pair_iterator(HglIniSection *section);

/**
 * Returns true if the section `section_name` exists in `ini`.
 */
bool hgl_ini_has_section(HglIni *ini, const char *section_name);

/**
 * Returns the section with the name `section_name`, or NULL if 
 * it doesn't exist.
 */
HglIniSection *hgl_ini_get_section(HglIni *ini, const char *section_name);

/**
 * Returns true if the `section_name` `key_name` combination exists in `ini`.
 */
bool hgl_ini_has(HglIni *ini, const char *section_name, const char *key_name);

/**
 * Gets a raw value from `ini`, i.e. a string representation of the value.
 */
const char *hgl_ini_get(HglIni *ini, const char *section_name, const char *key_name);

/**
 * Gets a raw value from `section`, i.e. a string representation of the value.
 */
const char *hgl_ini_get_in_section(HglIniSection *section, const char *key_name);

/**
 * Gets a boolean value from `ini`. The raw value is retrieved and parsed
 * as a boolean.
 *
 * Only the raw values "true", "True", "TRUE", and "1" represent a true value.
 * All other raw values result in this function returning false.
 */
bool hgl_ini_get_bool(HglIni *ini, const char *section_name, const char *key_name);

/**
 * Gets a signed integer value from `ini`. The raw value is retrieved and
 * parsed as a 64-bit signed integer.
 */
int64_t hgl_ini_get_i64(HglIni *ini, const char *section_name, const char *key_name);

/**
 * Gets an unsigned integer value from `ini`. The raw value is retrieved and
 * parsed as a 64-bit unsigned integer.
 */
uint64_t hgl_ini_get_u64(HglIni *ini, const char *section_name, const char *key_name);

/**
 * Gets an floating point value from `ini`. The raw value is retrieved and
 * parsed as a 64-bit float.
 *
 * Note: this assumes "double" is 64 bits, which it in most cases is.
 */
double hgl_ini_get_f64(HglIni *ini, const char *section_name, const char *key_name);

/**
 * Adds or updates a key-value pair in `ini`. If the section name or key name
 * is not present in `ini` a new key-value pair will be added. If both the
 * section name and key name are present in `ini` the old value will be updated.
 */
void hgl_ini_put(HglIni *ini, const char *section_name, const char *key_name, const char *raw_value);

/**
 * Pretty-prints the contents of `ini` on `stream`. The output is a valid *.ini file albeit
 * without any comments.
 */
void hgl_ini_fprint(FILE *stream, HglIni *ini);

#endif /* HGL_INI_H */

#ifdef HGL_INI_IMPLEMENTATION

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#if !defined(HGL_INI_ALLOC) &&   \
    !defined(HGL_INI_REALLOC) && \
    !defined(HGL_INI_FREE)
#include <stdlib.h>
#define HGL_INI_ALLOC malloc
#define HGL_INI_REALLOC realloc
#define HGL_INI_FREE free
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define hgl_ini_da_add(da, item)                                                         \
    do {                                                                                 \
        if ((da)->capacity < ((da)->count + 1)) {                                        \
            (da)->capacity = MAX(10, 2*(da)->capacity);                                  \
            (da)->arr = HGL_INI_REALLOC((da)->arr, (da)->capacity * sizeof(*(da)->arr)); \
        }                                                                                \
        assert(((da)->arr != NULL) && "[hgl] Error: (re)alloc failed");                  \
        (da)->arr[(da)->count++] = (item);                                               \
    } while (0)

#define hgl_ini_da_free(da)                                                              \
    do {                                                                                 \
        HGL_INI_FREE((da)->arr);                                                         \
    } while (0)

typedef struct
{
    const char *ptr;
    const char *const eof;
} HglIniCursor;

static char *str_duplicate(const char *str)
{
    size_t size = strlen(str);
    char *str_copy = HGL_INI_ALLOC(size + 1);
    assert(str_copy != NULL);
    memcpy(str_copy, str, size);
    str_copy[size] = '\0';
    return str_copy;
}

static bool cursor_reached_eof(HglIniCursor *cursor)
{
    return (cursor->ptr >= cursor->eof) || (*cursor->ptr == '\0');
}

static int step_cursor(HglIniCursor *cursor)
{
    if (!cursor_reached_eof(cursor)) {
        cursor->ptr++;
        return 0;
    }
    return 1;
}

static void ltrim(const char **str_start)
{
    char c = **str_start;
    while (c == ' '  || c == '\t' || c == '\n' ||
           c == '\r' || c == '\v' || c == '\f') {
        (*str_start)++;
        c = **str_start;
    }
}

static void rtrim(const char **str_end)
{
    char c = *(*str_end - 1);
    while (c == ' '  || c == '\t' || c == '\n' ||
           c == '\r' || c == '\v' || c == '\f') {
        (*str_end)--;
        c = *(*str_end - 1);
    }
}

static void eat_string_until(HglIniCursor *cursor, char end_char)
{
    while (*cursor->ptr != end_char) {
        if (cursor_reached_eof(cursor)) {
            return;
        }

        if (*cursor->ptr == '\\') {
            step_cursor(cursor);
        }

        step_cursor(cursor);
    }
}

static void eat_string_until_in_line(HglIniCursor *cursor, char end_char)
{
    while (*cursor->ptr != end_char && *cursor->ptr != '\n') {
        if (cursor_reached_eof(cursor)) {
            return;
        }

        if (*cursor->ptr == '\\') {
            step_cursor(cursor);
        }

        step_cursor(cursor);
    }
}

HglIni *hgl_ini_create(const char *filepath)
{
    HglIni *ini = HGL_INI_ALLOC(sizeof(HglIni));
    if (ini == NULL) {
        fprintf(stderr, "[hgl_ini_create] Error: Buy more ram lol\n");
        return NULL;
    }
    memset(ini, 0, sizeof(*ini));

    /* TODO ? */
    ini->filepath = str_duplicate(filepath);

    return ini;
}

HglIni *hgl_ini_open(const char *filepath)
{
    int fd = -1;
    char *data = MAP_FAILED;

    HglIni *ini = HGL_INI_ALLOC(sizeof(HglIni));
    if (ini == NULL) {
        fprintf(stderr, "[hgl_ini_open] Error: Buy more ram lol\n");
        goto out_error;
    }
    memset(ini, 0, sizeof(*ini));

    /* TODO ? */
    ini->filepath = str_duplicate(filepath);

    /* open file in read binary mode */
    fd = open(filepath, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "[hgl_ini_open] Error: errno=%s\n", strerror(errno));
        goto out_error;
    }

    /* get file size */
    struct stat sb;
    fstat(fd, &sb);
    if (fd == -1) {
        fprintf(stderr, "[hgl_ini_open] Error: errno=%s\n", strerror(errno));
        goto out_error;
    }

    /* mmap file */
    ssize_t file_size = sb.st_size;
    data = mmap(NULL,                   /* Let kernel choose page-aligned address */
                file_size,              /* Length of mapping in bytes */
                PROT_READ | PROT_WRITE, /* Readable & writable */
                MAP_SHARED,             /* Shared mapping. Visible to other processes */
                fd,                     /* File descriptor of file to map */
                0);                     /* Begin map at offset 0 into file */
    if (data == MAP_FAILED) {
        fprintf(stderr, "[hgl_ini_open] Error: mmap failed with errno=%s\n",
                strerror(errno));
        goto out_error;
    }

    /* parse ini file contents */
    HglIniCursor cursor = {
        .ptr = data,
        .eof = data + file_size,
    };
    int line_nr = 1;
    while (!cursor_reached_eof(&cursor)) {
        switch (*cursor.ptr) {

            case '[': {
                step_cursor(&cursor);
                const char *start = cursor.ptr;
                eat_string_until(&cursor, ']');
                const char *end = cursor.ptr;
                if (cursor_reached_eof(&cursor)) {
                    goto out_error;
                }
                step_cursor(&cursor);
                HglIniSection section = {0};
                section.name = HGL_INI_ALLOC(end - start + 1);
                memcpy(section.name, start, end - start);
                section.name[end - start] = '\0';
                hgl_ini_da_add(&ini->sections, section);
            } break;

            case  '\0': {
                fprintf(stderr, "[hgl_ini_open] Warning: Encountered \'\\0\' byte on line %d.\n", line_nr);
                goto out_error;
            } break;

            case '\n': {
                step_cursor(&cursor);
                line_nr++;
            } break;

            case  ' ': case '\r': case '\v':
            case '\t': case '\f': {
                step_cursor(&cursor);
            } break;

            case  ';': {
                eat_string_until(&cursor, '\n');
            } break;

            default: {
                /* find key */
                const char *key_start = cursor.ptr;
                eat_string_until_in_line(&cursor, '=');
                if (*cursor.ptr != '=') {
                    fprintf(stderr, "[hgl_ini_open] Error: Expected \'=\' on line %d.\n", line_nr);
                    goto out_error;
                }
                const char *key_end = cursor.ptr;
                step_cursor(&cursor);

                /* find value */
                const char *val_start = cursor.ptr;
                eat_string_until(&cursor, '\n');
                const char *val_end = cursor.ptr;
                step_cursor(&cursor);

                /* trim whitespace */
                ltrim(&key_start);
                ltrim(&val_start);
                rtrim(&key_end);
                rtrim(&val_end);

                HglIniKVPair kv_pair = (HglIniKVPair) {
                    .key = HGL_INI_ALLOC(key_end - key_start + 1),
                    .val = HGL_INI_ALLOC(val_end - val_start + 1),
                };
                memcpy(kv_pair.key, key_start, key_end - key_start);
                memcpy(kv_pair.val, val_start, val_end - val_start);
                kv_pair.key[key_end - key_start] = '\0';
                kv_pair.val[val_end - val_start] = '\0';

                if (ini->sections.capacity == 0) {
                    fprintf(stderr, "[hgl_ini_open] Error: Key-value pair does not belong to a section\n");
                    goto out_error;
                }

                HglIniSection *current_section = &ini->sections.arr[ini->sections.count - 1];
                hgl_ini_da_add(&current_section->kv_pairs, kv_pair);
            } break;
        }
    }

    close(fd);
    munmap(data, file_size);
    return ini;

out_error:
    if (fd != -1) {
        close(fd);
    }
    if (data != MAP_FAILED) {
        munmap(data, file_size);
    }
    hgl_ini_close(ini);
    return NULL;
}

void hgl_ini_save(HglIni *ini, const char *filepath)
{
    if (filepath != NULL) {
        HGL_INI_FREE(ini->filepath);
        ini->filepath = str_duplicate(filepath);
    }

    FILE *fp = fopen(ini->filepath, "wb");
    if (fp == NULL) {
        fprintf(stderr, "[hgl_ini_save] Error: Unable to save file. errno=%s\n", strerror(errno));
        goto out;
    }

    hgl_ini_fprint(fp, ini);

out:
    if (fp != NULL) {
        fclose(fp);
    }
}


void hgl_ini_close(HglIni *ini)
{
    for (size_t i = 0; i < ini->sections.count; i++) {
        HglIniSection *s = &ini->sections.arr[i];
        HGL_INI_FREE(s->name);
        for (size_t j = 0; j < s->kv_pairs.count; j++) {
            HglIniKVPair *kv = &s->kv_pairs.arr[j];
            HGL_INI_FREE(kv->key);
            HGL_INI_FREE(kv->val);
        }
        hgl_ini_da_free(&s->kv_pairs);
    }
    hgl_ini_da_free(&ini->sections);
    HGL_INI_FREE(ini->filepath);
    HGL_INI_FREE(ini);
}

HglIniSection *hgl_ini_next_section(HglIni *ini)
{
    if (ini->sections.iterator >= ini->sections.count) {
        return NULL;
    }
    return &ini->sections.arr[ini->sections.iterator++];
}

HglIniKVPair *hgl_ini_next_kv_pair(HglIniSection *section)
{
    if (section->kv_pairs.iterator >= section->kv_pairs.count) {
        return NULL;
    }
    return &section->kv_pairs.arr[section->kv_pairs.iterator++];
}

void hgl_ini_reset_section_iterator(HglIni *ini)
{
    ini->sections.iterator = 0;
}

void hgl_ini_reset_kv_pair_iterator(HglIniSection *section)
{
    section->kv_pairs.iterator = 0;
}

bool hgl_ini_has_section(HglIni *ini, const char *section_name)
{
    return (NULL != hgl_ini_get_section(ini, section_name));
}

HglIniSection *hgl_ini_get_section(HglIni *ini, const char *section_name)
{
    size_t section_name_len = strlen(section_name);

    for (size_t i = 0; i < ini->sections.count; i++) {
        HglIniSection *s = &ini->sections.arr[i];
        if (strncmp(section_name, s->name, section_name_len) == 0) {
            return s;
        }
    }

    return NULL;
}

bool hgl_ini_has(HglIni *ini, const char *section_name, const char *key_name)
{
    return (NULL != hgl_ini_get(ini, section_name, key_name));
}

const char *hgl_ini_get(HglIni *ini, const char *section_name, const char *key_name)
{
    size_t section_name_len = strlen(section_name);
    size_t key_name_len     = strlen(key_name);

    /* Get the raw value */
    for (size_t i = 0; i < ini->sections.count; i++) {
        HglIniSection *s = &ini->sections.arr[i];

        if (strncmp(section_name, s->name, section_name_len) != 0) {
            continue;
        }

        for (size_t j = 0; j < s->kv_pairs.count; j++) {
            HglIniKVPair *kv = &s->kv_pairs.arr[j];
            if (strncmp(key_name, kv->key, key_name_len) == 0) {
                return (const char *) kv->val;
            }
        }

        return NULL; /* don't allow duplicate section names */
    }
    return NULL;
}

const char *hgl_ini_get_in_section(HglIniSection *section, const char *key_name)
{
    size_t key_name_len = strlen(key_name);

    for (size_t j = 0; j < section->kv_pairs.count; j++) {
        HglIniKVPair *kv = &section->kv_pairs.arr[j];
        if (strncmp(key_name, kv->key, key_name_len) == 0) {
            return (const char *) kv->val;
        }
    }

    return NULL; /* don't allow duplicate section names */
}

bool hgl_ini_get_bool(HglIni *ini, const char *section_name, const char *key_name)
{
    const char *raw_value = hgl_ini_get(ini, section_name, key_name);
    size_t raw_value_len = strlen(raw_value);

    return ((raw_value_len == 4) && (strcmp(raw_value, "true") == 0)) ||
           ((raw_value_len == 4) && (strcmp(raw_value, "True") == 0)) ||
           ((raw_value_len == 4) && (strcmp(raw_value, "TRUE") == 0)) ||
           ((raw_value_len == 1) && (strcmp(raw_value, "1") == 0));
}

int64_t hgl_ini_get_i64(HglIni *ini, const char *section_name, const char *key_name)
{
    const char *raw_value = hgl_ini_get(ini, section_name, key_name);
    char *end;
    return strtol(raw_value, &end, 0);
}

uint64_t hgl_ini_get_u64(HglIni *ini, const char *section_name, const char *key_name)
{
    const char *raw_value = hgl_ini_get(ini, section_name, key_name);
    char *end;
    return strtoul(raw_value, &end, 0);
}

double hgl_ini_get_f64(HglIni *ini, const char *section_name, const char *key_name)
{
    const char *raw_value = hgl_ini_get(ini, section_name, key_name);
    char *end;
    return strtod(raw_value, &end);
}


void hgl_ini_put(HglIni *ini,
                 const char *section_name,
                 const char *key_name,
                 const char *raw_value)
{
    size_t section_name_len = strlen(section_name);
    size_t key_name_len     = strlen(key_name);
    size_t raw_value_len    = strlen(raw_value);

    HglIniSection *section = NULL;
    HglIniKVPair *kv_pair = NULL;

    /* look for already present section & kv-pair */
    for (size_t i = 0; i < ini->sections.count; i++) {
        HglIniSection *s = &ini->sections.arr[i];

        if (strcmp(section_name, s->name) != 0) {
            continue;
        }

        section = s;

        for (size_t j = 0; j < section->kv_pairs.count; j++) {
            HglIniKVPair *kv = &section->kv_pairs.arr[j];
            if (strcmp(kv->key, key_name) != 0) {
                continue;
            }
            kv_pair = kv;
            break;
        }

        break;
    }

    /* if section is not present, add it */
    if (section == NULL) {

        /* create new section */
        HglIniSection new_section = {0};
        new_section.name = HGL_INI_ALLOC(section_name_len + 1);
        memcpy(new_section.name, section_name, section_name_len);
        new_section.name[section_name_len] = '\0';

        /* Add to ini */
        hgl_ini_da_add(&ini->sections, new_section);
        section = &ini->sections.arr[ini->sections.count - 1];
    }

    assert(section != NULL);

    /* if kv-pair is not present, add it */
    if (kv_pair == NULL) {
        /* create new kv-pair */
        HglIniKVPair new_kv_pair = (HglIniKVPair) {
            .key = HGL_INI_ALLOC(key_name_len + 1),
            .val = NULL,
        };
        memcpy(new_kv_pair.key, key_name, key_name_len);
        new_kv_pair.key[key_name_len] = '\0';

        /* Add to section */
        hgl_ini_da_add(&section->kv_pairs, new_kv_pair);
        kv_pair = &section->kv_pairs.arr[section->kv_pairs.count - 1];
    }

    assert(kv_pair != NULL);

    HGL_INI_FREE(kv_pair->val);
    kv_pair->val = HGL_INI_ALLOC(raw_value_len + 1);
    memcpy(kv_pair->val, raw_value, raw_value_len);
    kv_pair->val[raw_value_len] = '\0';

}

void hgl_ini_fprint(FILE *stream, HglIni *ini)
{
    for (size_t i = 0; i < ini->sections.count; i++) {
        HglIniSection *s = &ini->sections.arr[i];
        fprintf(stream, "[%s]\n", s->name);
        for (size_t j = 0; j < s->kv_pairs.count; j++) {
            HglIniKVPair *kv = &s->kv_pairs.arr[j];
            fprintf(stream, "%s = %s\n", kv->key, kv->val);

        }
        fprintf(stream, "\n");
    }
    fflush(stream);
}

#endif

