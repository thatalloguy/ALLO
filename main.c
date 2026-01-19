

#include "Allo/chunk.h"
#include "Allo/virtual_machine.h"

int main(void) {
    init_vm();


    Chunk chunk;
    init_chunk(&chunk);

    // we are gonna compile the line. -((1.2 + 3.4) / 5.6)
    int constant = add_constant(&chunk, 1.2);
    write_chunk(&chunk, OP_CONSTANT, 123);
    write_chunk(&chunk, constant, 123);


    constant = add_constant(&chunk, 3.4);
    write_chunk(&chunk, OP_CONSTANT, 123);
    write_chunk(&chunk, constant, 123);

    write_chunk(&chunk, OP_ADD, 123);

    constant = add_constant(&chunk, 5.6);
    write_chunk(&chunk, OP_CONSTANT, 123);
    write_chunk(&chunk, constant, 123);

    write_chunk(&chunk, OP_DIVIDE, 123);
    write_chunk(&chunk, OP_NEGATE, 123);


    write_chunk(&chunk, OP_RETURN, 123);

   // disassemble_chunk(&chunk, "test chunk");
    interpret(&chunk);


    free_vm();
    free_chunk(&chunk);


    return 0;
}
