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

InterpretResult interpret_chunk(Chunk* chunk);
InterpretResult interpret_code(const char* source);

InterpretResult run();

void reset_stack();
void push_to_stack(Value value);
Value pop_stack();



#endif
