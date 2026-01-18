#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

void* reallocate(void* pointer, size_t oldSize,size_t newSize) {
    if (newSize == 0) {
        free(pointer);
        pointer = NULL;

        return NULL;
    }

    void* result = realloc(pointer, newSize);


    if (result == NULL) {
        printf("[memory] Ran into error while (re)allocating memory\n");
        exit(OUT_OF_MEMORY_CODE);
    }


    return result;
}
