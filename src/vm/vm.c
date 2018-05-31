#include <stdio.h>

#include "debug.h"
#include "value.h"
#include "vm.h"
#include "opcode.h"
#include "chunk.h"
#include "../compiler/compiler.h"

#define DEBUG_TRACE_EXECUTION 0

static InterpretResult run(Vm *vm, Chunk *chunk);

void init_vm(Vm *vm) {
    vm->sp = vm->stack;
    vm->chunk = NULL;
}

void free_vm(Vm *vm) {
    vm->chunk = NULL;
    vm->sp = NULL;
    vm->ip = NULL;
}

InterpretResult interpret(Vm *vm, const char *source) {
    Chunk chunk;
    init_chunk(&chunk);
    compile(source, &chunk);

    vm->ip = chunk.code;
    InterpretResult result = run(vm, &chunk);
    vm->chunk = NULL;

    free_chunk(&chunk);

    return result;
}

static InterpretResult run(Vm *vm, register Chunk *chunk) {
    register uint8_t *ip = vm->ip;
    register Value *sp = vm->sp;
    register Value *const_values = chunk->constants.values;

// Return byte at ip and advance ip
#define READ_BYTE() (*ip++)
#define READ_CONST() (const_values[READ_BYTE()])
#define READ_CONST_W() (const_values[(READ_BYTE() << 8) | READ_BYTE()])
#define READ_VAR() (chunk->variables.values[READ_BYTE()])
#define POP() (*(--sp))
#define PUSH(value) (*(sp++) = (value))
#define PEEK() (*(sp - 1))
#define BINARY_OP(op)                       \
    do {                                    \
        double second = POP();              \
        double first = POP();               \
        PUSH((first op second));            \
    } while (false)

#define COND_JUMP(op)                       \
    do {                                    \
        double second = POP();              \
        double first = POP();               \
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
        long stack_size = vm->sp - vm->stack;
        for (int i = 0; i < stack_size; ++i) {
            printf("%f\n", vm->stack[i]);
        }
        printf("sp: %li\n", stack_size);
        printf("ip: %li\n", ip - chunk->code);
        disassemble_instruction(chunk, ip - chunk->code);
#endif

        switch (instruction = READ_BYTE()) {
            case OP_RETURN:
                return INTERPRET_OK;
            case OP_LDC:
                // push(vm, READ_CONST());
                PUSH(READ_CONST());
                break;
            case OP_LDC_W: {
                // push(vm, READ_CONST_W());
                PUSH(READ_CONST_W());
                break;
            }
            case OP_LDC_0:
                // push(vm, 0);
                PUSH(0.0);
                break;
            case OP_LDC_1:
                // push(vm, 1);
                PUSH(1.0);
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
                //push(vm, -pop(vm));
                PUSH(-POP());
                break;
            case OP_LOAD:
                //push(vm, READ_VAR());
                PUSH(READ_VAR());
                break;
            case OP_STORE: {
                uint8_t index = READ_BYTE();
                write_at(&chunk->variables, index, POP());
                break;
            }
            case OP_POP:
                POP();
                break;
            case OP_PRINT:
                printf("%f\n", POP());
                break;
            case OP_DUP:
                PUSH(PEEK());
                break;
            case OP_JMP:
                ip = chunk->code + READ_BYTE();
                break;
            case OP_JEQ:
                COND_JUMP(==);
                break;
            case OP_JNE:
                COND_JUMP(!=);
                break;
            case OP_JLT:
                COND_JUMP(<);
                break;
            case OP_JLE:
                COND_JUMP(<=);
                break;
            case OP_JGT:
                COND_JUMP(>);
                break;
            case OP_JGE:
                COND_JUMP(>=);
                break;
            case OP_INC:
                PUSH(POP() + READ_BYTE());
                break;
            case OP_INC_1:
                PUSH(POP() + 1);
                break;
            case OP_DEC:
                PUSH(POP() - READ_BYTE());
                break;
            case OP_NOP:
                break;
            default:
                printf("Unknown instruction %d\n", instruction);
                return INTERPRET_RUNTIME_ERROR;
        }
    }

#undef COND_JUMP
#undef BINARY_OP
#undef PEEK
#undef PUSH
#undef POP
#undef READ_VAR
#undef READ_CONST
#undef READ_CONST_W
#undef READ_BYTE
}
