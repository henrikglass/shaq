/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2024 Henrik A. Glass
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
 * hgl_profile.h is a utility for quick and dirty profiling.
 *
 *
 * USAGE:
 *
 * Include hgl_hotload.h file like this:
 *
 *     #define HGL_PROFILE_IMPLEMENTATION
 *     #include "hgl_profile.h"
 *
 * If you want more detailed information, you can define:
 *
 *     #define HGL_PROFILE_DETAILED
 *
 * before including hgl_profile.h. NOTE: HGL_PROFILE_DETAILED is a work 
 * in progress and might not work at all.
 *
 * Profiling may be disabled for statements or groups of statements
 * passed as arguments HGL_PROFILE_MEASURE and HGL_PROFILE_MEASURE_NAMED
 * by defining:
 *
 *     HGL_PROFILE_DISABLED
 *
 * This could be useful if you want to disable profiling for certain builds
 * of your program.
 *
 *
 * EXAMPLE:
 *
 *     HGL_PROFILE_MEASURE(do_some_initializing());
 *     hgl_profile_report(0);
 *     hgl_profile_reset();
 *
 *     hgl_profile_begin("loop"); 
 *     for (int i = 0; i < 1000000; i++) {
 *         HGL_PROFILE_MEASURE(do_some_heavy_work());
 *         hgl_profile_begin("some-more-heavy-work");
 *         do_some_more_heavy_work();
 *         hgl_profile_end();
 *         hgl_profile_report(HGL_PROFILE_TIME_LAST); // report time elapsed for measurements this iteration
 *     }
 *     hgl_profile_end("loop"); 
 *     hgl_profile_report(HGL_PROFILE_EVERYTHING); // report everything (min, max, avg). If using HGL_PROFILE_DETAILED
 *                                                 // then this will also report branch misses, cache misses, etc.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */
 
#ifndef HGL_PROFILE_H
#define HGL_PROFILE_H

#include <stdint.h>
#include <stdbool.h>
#include <linux/perf_event.h>

#define HGL_PROFILE_TIME_LAST              (1u << 0)
#define HGL_PROFILE_TIME_AVG               (1u << 1)
#define HGL_PROFILE_TIME_MAX               (1u << 2)
#define HGL_PROFILE_TIME_TOTAL             (1u << 3)
#define HGL_PROFILE_CACHE_MISSES_LAST      (1u << 4)
#define HGL_PROFILE_CACHE_MISSES_AVG       (1u << 5)
#define HGL_PROFILE_CACHE_MISSES_TOTAL     (1u << 6)
#define HGL_PROFILE_BRANCH_MISSES_LAST     (1u << 7)
#define HGL_PROFILE_BRANCH_MISSES_AVG      (1u << 8)
#define HGL_PROFILE_BRANCH_MISSES_TOTAL    (1u << 9)

#define HGL_PROFILE_TIME_ALL          (HGL_PROFILE_TIME_LAST | \
                                       HGL_PROFILE_TIME_AVG  | \
                                       HGL_PROFILE_TIME_MAX  | \
                                       HGL_PROFILE_TIME_TOTAL)
#define HGL_PROFILE_CACHE_MISSES_ALL  (HGL_PROFILE_CACHE_MISSES_LAST | \
                                       HGL_PROFILE_CACHE_MISSES_AVG  | \
                                       HGL_PROFILE_CACHE_MISSES_TOTAL)
#define HGL_PROFILE_BRANCH_MISSES_ALL (HGL_PROFILE_BRANCH_MISSES_LAST | \
                                       HGL_PROFILE_BRANCH_MISSES_AVG  | \
                                       HGL_PROFILE_BRANCH_MISSES_TOTAL)
#define HGL_PROFILE_EVERYTHING        (HGL_PROFILE_TIME_ALL         | \
                                       HGL_PROFILE_CACHE_MISSES_ALL | \
                                       HGL_PROFILE_BRANCH_MISSES_ALL)
#define HGL_PROFILE_LAST              (HGL_PROFILE_TIME_LAST         | \
                                       HGL_PROFILE_CACHE_MISSES_LAST | \
                                       HGL_PROFILE_BRANCH_MISSES_LAST)
#define HGL_PROFILE_DEFAULT HGL_PROFILE_LAST

#ifndef HGL_PROFILE_DISABLED
#define HGL_PROFILE_MEASURE(stmt) do {hgl_profile_begin(#stmt); stmt; hgl_profile_end();} while(0)
#define HGL_PROFILE_MEASURE_NAMED(name, stmt) do {hgl_profile_begin(name); stmt; hgl_profile_end();} while(0)
#else
#define HGL_PROFILE_MEASURE(stmt) stmt
#define HGL_PROFILE_MEASURE_NAMED(name, stmt) stmt
#endif

typedef struct
{
    const char *name;
    uint64_t n_samples;
    uint32_t indent_level;
    bool is_complete;

    uint64_t time_ns_last;
    uint64_t time_ns_max;
    uint64_t time_ns_total;

#ifdef HGL_PROFILE_DETAILED
    uint64_t cache_misses_last;
    uint64_t cache_misses_total;
    uint64_t cache_references_last;
    uint64_t cache_references_total;

    uint64_t branch_misses_last;
    uint64_t branch_misses_total;
    uint64_t branches_last;
    uint64_t branches_total;

    uint64_t stalled_frontend_last;
    uint64_t stalled_frontend_total;
    uint64_t stalled_backend_last;
    uint64_t stalled_backend_total;

    uint64_t page_faults_last;
    uint64_t page_faults_total;

    uint64_t cycles_last;
    uint64_t cycles_total;

    uint64_t instructions_last;
    uint64_t instructions_total;

    struct perf_event_attr pe;
    int fd;
#endif
} HglProfStat;

void hgl_profile_begin(const char *name);
void hgl_profile_end(void);
void hgl_profile_report(uint32_t flags);
void hgl_profile_reset(void);
HglProfStat *hgl_profile_get(const char *name);
void hgl_profile_cleanup(void);

#endif /* HGL_PROFILE_H */

#ifdef HGL_PROFILE_IMPLEMENTATION

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <inttypes.h>
#include <errno.h>

#define Da_(T) \
    struct {               \
        T *arr;            \
        size_t length;     \
        size_t capacity;   \
    }

#define da_push_(da, item)                                                       \
    do {                                                                         \
        if ((da)->arr == NULL) {                                                 \
            (da)->length = 0;                                                    \
            (da)->capacity = 64;                                                 \
            (da)->arr = malloc((da)->capacity * sizeof(*(da)->arr));             \
        }                                                                        \
        if ((da)->capacity < ((da)->length + 1)) {                               \
            (da)->capacity *= 2;                                                 \
            (da)->arr = realloc((da)->arr, (da)->capacity * sizeof(*(da)->arr)); \
        }                                                                        \
        assert(((da)->arr != NULL) && "(re)alloc failed. Buy more RAM");         \
        (da)->arr[(da)->length++] = (item);                                      \
    } while (0)

#define da_pop_(da) ((da)->arr[--(da)->length])

uint64_t get_time_nanos_(void);

uint64_t get_time_nanos_()
{
    struct timespec t_temp;
    clock_gettime(CLOCK_MONOTONIC, &t_temp);
    uint64_t ns = t_temp.tv_sec * 1000000000ll + t_temp.tv_nsec;
    return ns;
}

#ifdef HGL_PROFILE_DETAILED
static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags);
static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}
#endif

Da_(HglProfStat) stats = {0};
Da_(const char *) stack = {0};

void hgl_profile_begin(const char *name)
{
    HglProfStat *entry = hgl_profile_get(name);
    if (entry == NULL) {
        HglProfStat new_entry = {0};
        new_entry.name = name;
        new_entry.time_ns_last  = 0;
        new_entry.time_ns_max   = 0;
        new_entry.time_ns_total = 0;
        da_push_(&stats, new_entry);
    }
    entry = hgl_profile_get(name);
    assert(entry != NULL);
    entry->indent_level = stack.length;
    da_push_(&stack, name);

    entry->is_complete = false;

#ifdef HGL_PROFILE_DETAILED
    entry->pe.type = PERF_TYPE_HW_CACHE;
    entry->pe.size = sizeof(struct perf_event_attr);
    entry->pe.config = PERF_COUNT_HW_CACHE_L1D |
                       PERF_COUNT_HW_CACHE_OP_READ << 8 |
                       PERF_COUNT_HW_CACHE_RESULT_MISS << 16;
    entry->pe.disabled = 1;
    entry->pe.exclude_kernel = 1;
    entry->pe.exclude_hv = 1;
    entry->fd = perf_event_open(&entry->pe, 0, -1, -1, 0);
    if (entry->fd == -1) {
        fprintf(stderr, "[hgl_prof]: Call to perf_event_open failed. Please make sure you are running\n"
                        "            the program in an environment that supports querying at least the\n"
                        "            following events (run `perf list`):\n"
                        "\n"
                        "            * branch-instructions OR branches\n"
                        "            * branch-misses\n"
                        "            * cache-misses\n"
                        "            * L1-dcache-load-misses\n"
                        "            * L1-dcache-loads\n"
                        "            * L1-dcache-store-misses\n"
                        "            * L1-dcache-stores\n"
                        "            * page-faults\n"
                        "            * cycles\n"
                        "            * instructions\n"
                        "            * stalled-cycles\n"
                        "\n"
                        "            These are typically not supported in a virtualized environment.\n"
                        "            Alternatively, you can omit HGL_PROFILE_DETAILED altogether.\n"
                        "\n"
                        "            If you are not running in a virtualized environment and you're\n"
                        "            still seeing this message, make sure you are running your program\n"
                        "            with the proper permissions (e.g. with sudo).\n\n"
                        );
        abort();
    }
    ioctl(entry->fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(entry->fd, PERF_EVENT_IOC_ENABLE, 0);
#endif

    entry->time_ns_last = get_time_nanos_();
}

void hgl_profile_end(void)
{
    uint64_t time_ns = get_time_nanos_();
    const char *name = da_pop_(&stack);
    HglProfStat *entry = hgl_profile_get(name);
    if (entry == NULL) abort(); // fix stupid warning
    entry->time_ns_last = time_ns - entry->time_ns_last;
    entry->time_ns_total += entry->time_ns_last;
    if (entry->time_ns_last > entry->time_ns_max) {
        entry->time_ns_max = entry->time_ns_last;
    }

#ifdef HGL_PROFILE_DETAILED
    ioctl(entry->fd, PERF_EVENT_IOC_DISABLE, 0);
    long long count;
    read(entry->fd, &count, sizeof(count));
    printf("count = %lld\n", count);
    close(entry->fd);
#endif

    entry->is_complete = true;
    entry->n_samples++;
}

void hgl_profile_report(uint32_t flags)
{
    if (flags == 0) {
        flags = HGL_PROFILE_DEFAULT;
    }

    for (uint32_t i = 0; i < stats.length; i++) {
        HglProfStat *entry = &stats.arr[i];
        int indent = (entry->indent_level + 1) * 1;

        if (!entry->is_complete) {
            printf("%*c[%s#%zu]: --- \n", indent, ' ', entry->name, entry->n_samples);
            continue;
        }

        printf("%*c[%s#%zu]: ", indent, ' ', entry->name, entry->n_samples);

        /* time */
        double last_ms = (double)entry->time_ns_last/1000000.0;
        double max_ms = (double)entry->time_ns_max/1000000.0;
        double total_ms = (double)entry->time_ns_total/1000000.0;
        double avg_ms = total_ms / entry->n_samples;
        if (flags & HGL_PROFILE_TIME_LAST) {
            printf("last = %f ms ", last_ms);
        }

        if (flags & HGL_PROFILE_TIME_AVG) {
            printf("avg = %f ms ", avg_ms);
        }

        if (flags & HGL_PROFILE_TIME_MAX) {
            printf("max = %f ms ", max_ms);
        }

        if (flags & HGL_PROFILE_TIME_TOTAL) {
            printf("total = %f ms ", total_ms);
        }

#ifdef HGL_PROFILE_DETAILED

#endif

        printf("\n");
    }
}

HglProfStat *hgl_profile_get(const char *name)
{
    for (uint32_t i = 0; i < stats.length; i++) {
        if (strcmp(stats.arr[i].name, name) == 0) {
            return &stats.arr[i];
        }
    }

    return NULL;
}

void hgl_profile_reset(void)
{
    stats.length = 0;
}


void hgl_profile_cleanup(void)
{
    free(stats.arr);
    free(stack.arr);
    stats.arr = NULL;
    stack.arr = NULL;
}

#endif

// TODO HGL_PROFILE_DISABLE - for all functions
