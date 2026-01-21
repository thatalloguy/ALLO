#include "virtual_machine.h"

#include <stdarg.h>
#include <stdio.h>

#include "debug.h"
#include "compiler.h"
VM vm;

void init_vm() {
    reset_stack();
}

void free_vm() {
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

static bool is_falsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void runtime_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);


    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    reset_stack();
}

InterpretResult interpret_chunk(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}

InterpretResult interpret_code(const char *source) {
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
#define NEGATE(ptr) (*(ptr-1) = NUMBER_VAL(-AS_NUMBER(*(ptr-1))))
#define BINARY_OP(valueType, op)                            \
    do {                                                    \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {     \
        runtime_error("Operands must be numbers.");          \
        return INTERPRET_RUNTIME_ERROR;                     \
      }                                                     \
      double b = AS_NUMBER(pop_stack());                          \
      double a = AS_NUMBER(pop_stack());                          \
      push_to_stack(valueType(a op b));                              \
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
                if (!IS_NUMBER(peek(0))) {
                    runtime_error("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                NEGATE(vm.stackTop);
                break;


            case OP_NIL: push_to_stack(NIL_VAL); break;
            case OP_TRUE: push_to_stack(BOOL_VAL(true)); break;
            case OP_FALSE: push_to_stack(BOOL_VAL(false)); break;

            case OP_ADD:        BINARY_OP(NUMBER_VAL, +); break;
            case OP_SUBTRACT:   BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY:   BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:     BINARY_OP(NUMBER_VAL, /); break;

            case OP_NOT: push_to_stack(BOOL_VAL(is_falsey(pop_stack()))); break;
            case OP_EQUAL: {
                Value b = pop_stack();
                Value a = pop_stack();
                push_to_stack(BOOL_VAL(values_equal(a, b)));
                break;
            }
            case OP_NOT_EQUAL: {
                Value b = pop_stack();
                Value a = pop_stack();
                push_to_stack(BOOL_VAL(!values_equal(a, b)));
                break;
            }


            case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
            case OP_GREATER_EQUAL: BINARY_OP(BOOL_VAL, >=); break;

            case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
            case OP_LESS_EQUAL: BINARY_OP(BOOL_VAL, <=); break;
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
