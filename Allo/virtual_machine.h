#ifndef allo_vm_h
#define allo_vm_h

#include "chunk.h"

//todo support multiple virtual machines?
typedef struct {
    Chunk* chunk;
    uint8_t* ip; // instruction pointer
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void init_vm();
void free_vm();

InterpretResult interpret(Chunk* chunk);
InterpretResult run();




#endif
