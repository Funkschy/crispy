#include <stdio.h>

#include "debug.h"
#include "value.h"
#include "vm.h"
#include "opcode.h"

#define DEBUG_TRACE_EXECUTION 0

void init_vm(Vm *vm) {
    vm->sp = vm->stack;
}

void free_vm(Vm *vm) {
}

inline void push(Vm *vm, Value value) {
    *vm->sp++ = value;
}

inline Value pop(Vm *vm) {
    return *(--vm->sp);
}

inline Value peek(Vm *vm) {
    return *(vm->sp - 1);
}

static InterpretResult run(Vm *vm) {
    register uint8_t *ip = vm->ip;
    Chunk *chunk = vm->chunk;

// Return byte at ip and advance ip
#define READ_BYTE() (*ip++)
#define READ_CONST() (chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)                \
    do {                             \
        double first = pop(vm);      \
        double second = pop(vm);     \
        push(vm, (first op second)); \
    } while (false)

#define JUMP(op)                            \
    do {                                    \
        uint8_t second = pop(vm);           \
        uint8_t first = pop(vm);            \
        if (first op second) {              \
            ip = chunk->code + READ_BYTE(); \
        } else {                            \
            READ_BYTE();                    \
        }                                   \
    } while(false)

    while (true) {
        uint8_t instruction;

#if DEBUG_TRACE_EXECUTION
        printf("-----\n");
        int stack_size = vm->sp - vm->stack;
        for (int i = 0; i < stack_size; ++i) {
            printf("%f\n", vm->stack[i]);
        }
        printf("sp: %d\n", vm->sp - vm->stack);
        disassemble_instruction(chunk, ip - chunk->code);
#endif

        switch (instruction = READ_BYTE()) {
            case OP_RETURN:
                return INTERPRET_OK;
            case OP_LDC:
                push(vm, READ_CONST());
                break;
            case OP_LDC_0:
                push(vm, 0);
                break;
            case OP_LDC_1:
                push(vm, 1);
                break;
            case OP_ADD:
                BINARY_OP(+);
                break;
            case OP_SUB:
                BINARY_OP(-);
                break;
            case OP_MUL:
                BINARY_OP(*);
                break;
            case OP_DIV:
                BINARY_OP(/);
                break;
            case OP_NEGATE:
                push(vm, -pop(vm));
                break;
            case OP_LOAD:
                push(vm, chunk->variables.values[READ_BYTE()]);
                break;
            case OP_STORE: {
                uint8_t index = READ_BYTE();
                write_at(&chunk->variables, index, pop(vm));
                break;
            }
            case OP_POP:
                pop(vm);
                break;
            case OP_PRINT:
                printf("%f\n", pop(vm));
                break;
            case OP_DUP:
                push(vm, peek(vm));
                break;
            case OP_JMP:
                ip = chunk->code + READ_BYTE();
                break;
            case OP_JEQ:
                JUMP(==);
                break;
            case OP_JNE:
                JUMP(!=);
                break;
            case OP_JLT:
                JUMP(<);
                break;
            case OP_JLE:
                JUMP(<=);
                break;
            case OP_JGT:
                JUMP(>);
                break;
            case OP_JGE:
                JUMP(>=);
                break;
            case OP_INC:
                push(vm, pop(vm) + READ_BYTE());
                break;
            case OP_DEC:
                push(vm, pop(vm) - READ_BYTE());
                break;
            default:
                printf("Unknown instruction %d\n", instruction);
                return INTERPRET_RUNTIME_ERROR;
        }
    }

#undef JUMP
#undef BINARY_OP
#undef READ_CONST
#undef READ_BYTE
}

InterpretResult interpret(Vm *vm, Chunk *chunk) {
    vm->chunk = chunk;
    vm->ip = vm->chunk->code;

    InterpretResult result = run(vm);

    return result;
}