#ifndef allo_memory_h
#define allo_memory_h

#include "common.h"

#define GROW_CAPACITY(capacity) { \
    ((capacity < 8 ? 8 : (capacity) * 2))

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))

#endif //allo_memory_h
