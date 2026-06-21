#ifndef allo_vm_h
#define allo_vm_h

#include "chunk.h"
#include "object.h"
#include "table.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)


typedef struct {
    ObjFunction* function;
    uint8_t* ip; //instruction pointer
    Value* slots; // array
} CallFrame;


//todo support multiple virtual machines?
typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack[STACK_MAX];
    Value* stackTop;
    Table strings;
    Table globals;
    Obj* objects;
} VM;


typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;


void init_vm();
void free_vm();

InterpretResult interpret_code(const char* source);

void define_native(const char* name, NativeFn function);



InterpretResult run();

void reset_stack();
void push_to_stack(Value value);
Value pop_stack();



#endif
