#ifndef ARRAY_H
#define ARRAY_H

#include <sys/types.h>

#define Array(T, N)        \
    struct {               \
        T arr[N];          \
        u32 count;         \
    }

#define array_push(array_, item_) \
    do { \
        (array_)->arr[(array_)->count++] = (item_); \
    } while (0) \

#define array_delete(array_, index_) \
    do { \
        (array_)->arr[(index_)] = (array_)->arr[(array_)->count - 1]; \
        (array_)->count--; \
    } while (0) \

#define array_clear(array_) (array_)->count = 0

#endif /* ARRAY_H */

