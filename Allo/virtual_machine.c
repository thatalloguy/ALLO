#include "virtual_machine.h"

#include <stdio.h>

#include "debug.h"

VM vm;

void init_vm() {

}

void free_vm() {
}


InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}

InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    for (;;) {

#ifdef ALLO_DEBUG_TRACE_EXECUTION
        disassemble_instruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_RETURN:
                return INTERPRET_OK;
            case OP_CONSTANT:
                Value constant = READ_CONSTANT();
                print_value(constant);
                printf("\n");
                break;
            default:
                return INTERPRET_COMPILE_ERROR;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
}
