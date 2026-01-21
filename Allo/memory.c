#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

#include "object.h"
#include "value.h"
#include "virtual_machine.h"

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

static void free_object(Obj* obj) {
    switch (obj->type) {
        case OBJ_STRING:
            ObjString* string = (ObjString*)obj;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjString, obj);
            break;
    }
}

void free_objects() {
    Obj* object = vm.objects;
    while (object != NULL) {
        Obj* next = object->next;
        free_object(object);
        object = next;
    }
}
