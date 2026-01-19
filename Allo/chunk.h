#ifndef allo_chunk_h
#define allo_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,

    //binary operators
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,

    OP_RETURN,
} OpCode;

typedef struct {
    uint8_t* code;
    //todo optimize this.
    int* lines;
    ValueArray constants;
    int count;
    int capacity;
} Chunk;

void init_chunk(Chunk* chunk);
void free_chunk(Chunk* chunk);

void write_chunk(Chunk* chunk, uint8_t byte, int line);
int add_constant(Chunk* chunk, Value value);




#endif
