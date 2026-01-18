#include "Allo/chunk.h"

int main(void) {

    Chunk chunk;
    init_chunk(&chunk);

    write_chunk(&chunk, OP_RETURN);

    free_chunk(&chunk);

    return 0;
}