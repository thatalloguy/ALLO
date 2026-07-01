#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "object.h"
#include "value.h"
#include "virtual_machine.h"

#ifdef ALLO_DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

void* reallocate(void* pointer, size_t oldSize,size_t newSize) {
    vm.bytesAllocated += newSize - oldSize;
#ifdef ALLO_DEBUG_STRESS_GC

    if (newSize > oldSize) {
        collect_garbage();
    }
#endif

    if (vm.bytesAllocated > vm.nextGC) {
        collect_garbage();
    }

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
#ifdef ALLO_DEBUG_LOG_GC
    printf("%p free type %d\n", (void*)obj, obj->type);
#endif

    switch (obj->type) {
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)obj;
            free_chunk(&function->chunk);
            FREE(ObjFunction, obj);
            break;
        }
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)obj;
            FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalueCount);
            FREE(ObjClosure, obj);
            break;
        }

        case OBJ_CLASS: {
            ObjClass* klass = (ObjClass*) obj;
            free_table(&klass->methods);
            FREE(ObjClass, obj);
            break;
        }


        case OBJ_UPVALUE: {
            FREE(ObjUpvalue, obj);
            break;
        }

        case OBJ_NATIVE: {
            FREE(ObjNative, obj);
            break;
        }
        case OBJ_STRING: {
            ObjString* string = (ObjString*)obj;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjString, obj);
            break;
        }

        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*) obj;
            free_table(&instance->fields);
            FREE(ObjInstance, obj);
            break;
        }

        case OBJ_BOUND_METHOD: {
            FREE(ObjBoundMethod, obj);
        }
    }
}

void mark_object(Obj* object) {
    if (object == NULL) return;
    if (object->isMarked) return;

#ifdef ALLO_DEBUG_LOG_GC
    printf("%p mark ", (void*) object);
    print_value(OBJ_VAL(object));
    printf("\n");
#endif

    object->isMarked = true;

    if (vm.grayCapacity < vm.grayCount + 1) {
        vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
        vm.grayStack = (Obj**) realloc(vm.grayStack, sizeof(Obj*) * vm.grayCapacity);

        if (vm.grayStack == NULL) exit(1);

    }

    vm.grayStack[vm.grayCount++] = object;
}

void mark_value(Value value) {
    if (IS_OBJ(value)) mark_object(AS_OBJ(value));
}

static void mark_array(ValueArray* array) {
    for (int i=0; i<array->count; i++) {
        mark_value(array->values[i]);
    }
}

static void mark_roots() {
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
        mark_value(*slot);
    }

    for (int i=0; i<vm.frameCount; i++) {
        mark_object((Obj*)vm.frames[i].closure);
    }

    for (ObjUpvalue* upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next) {
        mark_object((Obj*)upvalue);
    }

    mark_table(&vm.globals);
    mark_compiler_roots();
    mark_object((Obj*)vm.initString);
}

static void blacken_object(Obj* object) {
#ifdef ALLO_DEBUG_LOG_GC
    printf("%p blacken ", (void*)object);
    print_value(OBJ_VAL(object));
    printf("\n");
#endif

    switch (object->type) {
        case OBJ_NATIVE:
        case OBJ_STRING:
            break;

        case OBJ_UPVALUE:
            mark_value(((ObjUpvalue*)object)->closed);
            break;

        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*) object;
            mark_object((Obj*)function->name);
            mark_array(&function->chunk.constants);
            break;
        }

        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*) object;
            mark_object((Obj*)closure->function);
            for (int i=0; i<closure->upvalueCount; i++) {
                mark_object((Obj*) closure->upvalues[i]);
            }
            break;
        }

        case OBJ_CLASS: {
            ObjClass* klass = (ObjClass*)object;
            mark_object((Obj*)klass->name);
            mark_table(&klass->methods);
            break;
        }

        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*) object;
            mark_object((Obj*) instance->klass);
            mark_table(&instance->fields);
            break;
        }

        case OBJ_BOUND_METHOD: {
            ObjBoundMethod* bound = (ObjBoundMethod*)object;
            mark_value(bound->receiver);
            mark_object((Obj*)bound->method);

            break;
        }
    }
}

static void trace_references() {
    while (vm.grayCount > 0) {
        Obj* object = vm.grayStack[--vm.grayCount];
        blacken_object(object);
    }
}

static void sweep() {
    Obj* previous = NULL;
    Obj* object = vm.objects;
    while (object != NULL) {
        if (object->isMarked) {
            object->isMarked = false;
            previous = object;
            object = object->next;
        } else {
            Obj* unreached = object;
            object = object->next;
            if (previous != NULL) {
                previous->next = object;
            } else {
                vm.objects = object;
            }

            free_object(unreached);
        }
    }
}

void collect_garbage() {
#ifdef ALLO_DEBUG_LOG_GC
    printf("-- gc begin\n");
    size_t before = vm.bytesAllocated;
#endif

    mark_roots();
    trace_references();
    table_remove_white(&vm.strings);
    sweep();


    vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef ALLO_DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
         before - vm.bytesAllocated, before, vm.bytesAllocated,
         vm.nextGC);

#endif
}


void free_objects() {
    Obj* object = vm.objects;
    while (object != NULL) {
        Obj* next = object->next;
        free_object(object);
        object = next;
    }

    free(vm.grayStack);
}
