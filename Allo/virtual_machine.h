#ifndef allo_vm_h
#define allo_vm_h

#include "chunk.h"

#define STACK_MAX 256



//todo support multiple virtual machines?
typedef struct {
    Chunk* chunk;
    uint8_t* ip; // instruction pointer
    Value stack[STACK_MAX];
    Value* stackTop;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void init_vm();
void free_vm();

InterpretResult interpret(Chunk* chunk);
InterpretResult interpret(const char* source);

InterpretResult run();

void reset_stack();
void push_to_stack(Value value);
Value pop_stack();



#endif
