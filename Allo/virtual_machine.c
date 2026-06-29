#include "virtual_machine.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "debug.h"
#include "compiler.h"
#include "memory.h"
#include "object.h"
VM vm;


static Value clockNative(int argCount, Value* args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

void init_vm() {
    reset_stack();
    vm.objects = NULL;
    vm.openUpvalues = NULL;
    init_table(&vm.strings);
    init_table(&vm.globals);

    define_native("clock", clockNative);
}

void free_vm() {
    free_table(&vm.strings);
    free_table(&vm.globals);
    free_objects();
}


static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}



static bool is_falsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
    ObjString* b = AS_STRING(pop_stack());
    ObjString* a = AS_STRING(pop_stack());

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = take_string(chars, length);
    push_to_stack(OBJ_VAL(result));
}

static void runtime_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i=vm.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        int line = frame->closure->function->chunk.lines[instruction];

        fprintf(stderr, "[line %d] in script\n", line);

        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr,"%s()\n", function->name->chars);
        }

    }

    reset_stack();
}
static bool call(ObjClosure* closure, int argCount) {
    ObjFunction* function = closure->function;

    if (argCount != function->arity) {
        runtime_error("Excepted %d arguments but got %d", function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX) {
        runtime_error("STACK OVERFLOW <-- HOW DID YOU DO THAT YOU IDIOT!");
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}


static bool call_value(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch OBJ_TYPE(callee) {
            // case OBJ_FUNCTION:
            //     return call(AS_FUNCTION(callee), argCount);
            case OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                push_to_stack(result);

                return true;
            }
            case OBJ_CLOSURE: {
                return call(AS_CLOSURE(callee), argCount);
            }
            default:
                break;
        }
    }

    runtime_error("Can only call functions and classes");

    return false;
}

static ObjUpvalue* capture_upvalue(Value* local) {

    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue* createdUpvalue = new_upvalue(local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }
    return createdUpvalue;
}


static void close_upvalues(Value* last) {
    while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

void define_native(const char* name, NativeFn function) {
    push_to_stack(OBJ_VAL(copy_string(name, (int)strlen(name))));
    push_to_stack(OBJ_VAL(new_native(function)));
    table_set(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop_stack();
    pop_stack();
}

InterpretResult interpret_code(const char *source) {
    ObjFunction* function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push_to_stack(OBJ_VAL(function));
    ObjClosure* closure = new_closure(function);
    pop_stack();
    push_to_stack(OBJ_VAL(closure));
    call(closure, 0);

    return run();
}

InterpretResult run() {
    CallFrame* frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)

#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_BYTE()])

#define READ_STRING() \
    AS_STRING(READ_CONSTANT())

#define READ_SHORT() \
    (frame->ip += 2, \
    (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1])) // reads 2 bytes.

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

        disassemble_instruction(&frame->closure->function->chunk,
            (int)(frame->ip - frame->function->chunk.code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_RETURN: {
                Value result = pop_stack();
                close_upvalues(frame->slots);
                vm.frameCount--;
                if (vm.frameCount == 0) {
                    pop_stack();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                push_to_stack(result);
                frame = &vm.frames[vm.frameCount - 1];

                break;
            }
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

            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double a = AS_NUMBER(pop_stack());
                    double b = AS_NUMBER(pop_stack());
                    push_to_stack(NUMBER_VAL(a + b));
                } else {
                    runtime_error("Operands must be two numbers or two strings");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
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

                //---
            case OP_PRINT:
                print_value(pop_stack());
                printf("\n");
                break;
            case OP_POP: pop_stack(); break;
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                table_set(&vm.globals, name, peek(0));
                pop_stack();
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if (!table_get(&vm.globals, name, &value)) {
                    runtime_error("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push_to_stack(value);
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (table_set(&vm.globals, name, peek(0))) {
                    table_delete(&vm.globals, name);
                    runtime_error("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push_to_stack(frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                // if (is_falsey(peek(0))) vm.ip += offset;
                frame->ip += offset * is_falsey(peek(0));
                break;
            }

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }

            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }

            case OP_CALL: {
                int argCount = READ_BYTE();
                if  (!call_value(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }

            case OP_CLOSURE: {
                ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = new_closure(function);
                push_to_stack(OBJ_VAL(closure));

                for (int i =0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    closure->upvalues[i] = isLocal ?
                                capture_upvalue(frame->slots + index)
                                : frame->closure->upvalues[index];
                }

                break;
            }

            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push_to_stack(*frame->closure->upvalues[slot]->location);
                break;
            }

            case OP_CLOSE_UPVALUE: {
                close_upvalues(vm.stackTop - 1);
                pop_stack();
                break;
            }

            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(0);
                break;
            }

            default:
                return INTERPRET_COMPILE_ERROR;
        }
    }

#undef BINARY_OPERATOR
#undef READ_BYTE
#undef READ_STRING
#undef READ_SHORT
#undef READ_CONSTANT
}

void reset_stack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
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
