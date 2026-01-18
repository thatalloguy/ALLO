#ifndef allo_chunk_h
#define allo_chunk_h

#include "common.h"

#include "memory.h"

typedef enum {
    OP_RETURN,
} OpCode;

typedef struct {
    uint8_t* code;
    int count;
    int capacity;
} Chunk;

void init_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t byte);




#endif
