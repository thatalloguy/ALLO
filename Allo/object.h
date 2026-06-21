#ifndef allo_object_h
#define allo_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"

#define OBJ_TYPE(value)         (AS_OBJ(value)->type)

#define IS_STRING(value)        is_obj_type(value, OBJ_STRING)
#define IS_FUNCTION(value)     is_obj_type(value, OBJ_FUNCTION)
#define IS_NATIVE(value)        is_obj_type(value, OBJ_NATIVE)

#define AS_NATIVE(value)        ((ObjNative*)AS_OBJ(value))->function
 #define AS_FUNCTION(value)     ((ObjFunction*)AS_OBJ(value))
#define AS_STRING(value)        ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)       (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
} ObjType;


struct Obj {
    ObjType type;
    Obj* next;
};

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

typedef struct {
    Obj obj;
    int arity; // number of parameters.
    Chunk chunk;
    ObjString* name;
} ObjFunction;


struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};


ObjFunction* new_function();
ObjNative* new_native(NativeFn function);
ObjString* copy_string(const char* chars, int length);
ObjString* take_string(char* chars, int length);

void print_object(Value value);

static inline bool is_obj_type(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}



#endif