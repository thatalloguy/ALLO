#ifndef allo_object_h
#define allo_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

#define OBJ_TYPE(value)         (AS_OBJ(value)->type)

#define IS_class(value)        is_obj_type(value, OBJ_CLASS)
#define IS_STRING(value)        is_obj_type(value, OBJ_STRING)
#define IS_FUNCTION(value)     is_obj_type(value, OBJ_FUNCTION)
#define IS_INSTANCE(value)      is_obj_type(value, OBJ_INSTANCE)
#define IS_NATIVE(value)        is_obj_type(value, OBJ_NATIVE)
#define IS_CLOSURE(value)       is_obj_type(value, OBJ_CLOSURE)
#define IS_BOUND_METHOD(value)       is_obj_type(value, OBJ_BOUND_METHOD)

#define AS_BOUND_METHOD(value)       ((ObjBoundMethod*)AS_OBJ(value))
#define AS_CLASS(value)       ((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value)      ((ObjInstance*)AS_OBJ(value))
#define AS_CLOSURE(value)       ((ObjClosure*)AS_OBJ(value))
#define AS_NATIVE(value)        ((ObjNative*)AS_OBJ(value))->function
#define AS_FUNCTION(value)     ((ObjFunction*)AS_OBJ(value))
#define AS_STRING(value)        ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)       (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_BOUND_METHOD,
    OBJ_CLASS,
    OBJ_INSTANCE,
    OBJ_FUNCTION,
    OBJ_CLOSURE,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE,
} ObjType;


struct Obj {
    ObjType type;
    bool isMarked;
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
    int upvalueCount;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

typedef struct ObjUpvalue {
    Obj obj;
    Value* location;
    Value closed;
    struct ObjUpvalue* next;
} ObjUpvalue;



typedef struct {
    Obj obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;
} ObjClosure;


struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};


typedef struct {
    Obj obj;
    ObjString* name;
    Table methods;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass* klass;
    Table fields;
} ObjInstance;

typedef struct {
    Obj obj;
    Value receiver;
    ObjClosure* method;
} ObjBoundMethod;

ObjBoundMethod* new_bound_method(Value receiver, ObjClosure* method);
ObjInstance* new_instance(ObjClass* klass);
ObjClass* new_class(ObjString* name);
ObjFunction* new_function();
ObjClosure* new_closure(ObjFunction* function);
ObjUpvalue* new_upvalue(Value* slot);
ObjNative* new_native(NativeFn function);

ObjString* copy_string(const char* chars, int length);
ObjString* take_string(char* chars, int length);

void print_object(Value value);

static inline bool is_obj_type(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}



#endif