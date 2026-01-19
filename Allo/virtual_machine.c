#include "virtual_machine.h"

#include <stdio.h>

#include "debug.h"

VM vm;

void init_vm() {
    reset_stack();
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

        printf("        ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            print_value(*slot);
            printf(" ]");
        }
        printf("\n");

        disassemble_instruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_RETURN:
                print_value(pop_stack());
                printf("\n");
                return INTERPRET_OK;
            case OP_CONSTANT:
                Value constant = READ_CONSTANT();
                push_to_stack(constant);
                break;
            default:
                return INTERPRET_COMPILE_ERROR;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
}

void reset_stack() {
    vm.stackTop = vm.stack;
}

void push_to_stack(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;


    //todo implement  proper  stack overflow warnings / errors;
    if (vm.stackTop - vm.stack >= STACK_MAX) {
        printf("[WARNING] STACK IS OVERFLOWING\n");
    }
}


Value pop_stack() {
    vm.stackTop--;
    return *vm.stackTop;
}
