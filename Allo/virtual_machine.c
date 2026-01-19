#include "virtual_machine.h"

#include <stdio.h>

#include "debug.h"
#include "compiler.h"
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

InterpretResult interpret(const char *source) {
    Chunk chunk;
    init_chunk(&chunk);

    if (!compile(source, &chunk)) {
        free_chunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    free_chunk(&chunk);

    return result;
}

InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define NEGATE(ptr) (*(ptr-1) = -*(ptr-1))
#define BINARY_OPERATOR(op)         \
        do {                        \
            double a = pop_stack(); \
            double b = pop_stack(); \
            push_to_stack(a op b);   \
        } while (false)



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

                //---- Binary operators
            case OP_NEGATE:
                NEGATE(vm.stackTop);
                break;
            case OP_ADD:        BINARY_OPERATOR(+); break;
            case OP_SUBTRACT:   BINARY_OPERATOR(-); break;
            case OP_MULTIPLY:   BINARY_OPERATOR(*); break;
            case OP_DIVIDE:     BINARY_OPERATOR(/); break;

                //----
            case OP_CONSTANT:
                Value constant = READ_CONSTANT();
                push_to_stack(constant);
                break;
            default:
                return INTERPRET_COMPILE_ERROR;
        }
    }

#undef BINARY_OPERATOR
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
