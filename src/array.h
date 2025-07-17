#ifndef ARRAY_H
#define ARRAY_H

#define Array(T, N)        \
    struct {               \
        T arr[N];          \
        size_t count;      \
    }

#define array_push(array_, item_) \
    do { \
        (array_)->arr[(array_)->count++] = (item_); \
    } while (0) \

#define array_clear(array_) (array_)->count = 0

#endif /* ARRAY_H */

