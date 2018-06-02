#include <stdio.h>

#include "debug.h"
#include "vm.h"
#include "opcode.h"
#include "../compiler/compiler.h"
#include "memory.h"

static InterpretResult run(Vm *vm);

void init_vm(Vm *vm) {
    vm->sp = vm->stack;
    vm->first_object = NULL;
    vm->num_objects = 0;
    vm->max_objects = INITIAL_GC_THRESHOLD;
    vm->instruction_cap = 0;
    vm->instruction_count = 0;
    vm->code = NULL;

    init_value_array(&vm->constants);
    init_value_array(&vm->variables);
}

void free_vm(Vm *vm) {
    gc(vm);

    vm->sp = NULL;
    vm->ip = NULL;
    vm->first_object = NULL;
    vm->num_objects = 0;
    vm->max_objects = 0;

    free_value_array(&vm->constants);
    free_value_array(&vm->variables);

    free(vm->code);
}

void write_code(Vm *vm, uint8_t instruction) {
    if (vm->instruction_count >= vm->instruction_cap) {
        vm->instruction_cap = GROW_CAP(vm->instruction_cap);
        vm->code = GROW_ARR(vm->code, uint8_t, vm->instruction_cap);
    }

    vm->code[vm->instruction_count++] = instruction;
}

uint32_t add_constant(Vm *vm, Value value) {
    write_value(&vm->constants, value);
    return vm->constants.count - 1;
}

InterpretResult interpret(Vm *vm, const char *source) {
    compile(source, vm);

#if DEBUG_SHOW_DISASSEMBLY
    disassemble_vm(vm, "program");
#endif

    vm->ip = vm->code;
    InterpretResult result = run(vm);

    return result;
}

static InterpretResult run(Vm *vm) {
    register uint8_t *ip = vm->ip;
    register Value *sp = vm->sp;
    register Value *const_values = vm->constants.values;

// Return byte at ip and advance ip
#define READ_BYTE() (*ip++)
#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
#define READ_CONST() (const_values[READ_BYTE()])
#define READ_CONST_W() (const_values[(READ_BYTE() << 8) | READ_BYTE()])
#define READ_VAR() (vm->variables.values[READ_BYTE()])
#define POP() (*(--sp))
#define PUSH(value) (*(sp++) = (value))
#define PEEK() (*(sp - 1))
#define BINARY_OP(op)                                           \
    do {                                                        \
        Value second = POP();                                   \
        Value first = POP();                                    \
        if (!CHECK_NUM(first) || !CHECK_NUM(second))            \
            goto ERROR;                                         \
        PUSH(create_number(first.d_value op second.d_value));   \
    } while (false)

#define BOOL_OP(op)                                             \
    do {                                                        \
        Value second = POP();                                   \
        Value first = POP();                                    \
        PUSH(create_bool(first.d_value op second.d_value));     \
    } while (false)

#define COND_JUMP(op)                                   \
    do {                                                \
        Value second = POP();                           \
        Value first = POP();                            \
        if (!CHECK_BOOL(first) || !CHECK_BOOL(second))  \
            goto ERROR;                                 \
        if (first.p_value op second.p_value) {          \
            ip = vm->code + READ_SHORT();               \
        } else {                                        \
            READ_SHORT();                               \
        }                                               \
    } while(false)

    while (true) {
        OP_CODE instruction;

#if DEBUG_TRACE_EXECUTION
        printf("-----\n");
        long stack_size = sp - vm->stack;
        for (int i = 0; i < stack_size; ++i) {
            print_value(vm->stack[i], false);
            print_type(vm->stack[i]);
        }
        printf("sp: %li\n", stack_size);
        printf("ip: %li\n", ip - vm->code);
        disassemble_instruction(vm, ip - vm->code);
#endif

        switch (instruction = (OP_CODE) READ_BYTE()) {
            case OP_RETURN:
                return INTERPRET_OK;
            case OP_LDC:
                PUSH(READ_CONST());
                break;
            case OP_LDC_W: {
                PUSH(READ_CONST_W());
                break;
            }
            case OP_LDC_0: {
                Value zero = create_number(0.0);
                PUSH(zero);
                break;
            }
            case OP_LDC_1: {
                Value one = create_number(1.0);
                PUSH(one);
                break;
            }
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
            case OP_EQUAL:
                BOOL_OP(==);
                break;
            case OP_NOT_EQUAL:
                BOOL_OP(!=);
                break;
            case OP_GREATER:
                BOOL_OP(<=);
                break;
            case OP_LESS:
                BOOL_OP(>=);
                break;
            case OP_NEGATE: {
                Value val = create_number(POP().d_value * -1);
                PUSH(val);
                break;
            }
            case OP_LOAD:
                PUSH(READ_VAR());
                break;
            case OP_STORE: {
                uint8_t index = READ_BYTE();
                write_at(&vm->variables, index, POP());
                break;
            }
            case OP_POP:
                POP();
                break;
            case OP_PRINT:
                print_value(POP(), true);
                break;
            case OP_DUP:
                PUSH(PEEK());
                break;
            case OP_JMP:
                ip = vm->code + READ_SHORT();
                break;
            case OP_JMT: {
                Value value = POP();
                if (!CHECK_BOOL(value)) goto ERROR;
                if (BOOL_TRUE(value)) {
                    ip = vm->code + READ_SHORT();
                } else {
                    ip += 2;
                }
                break;
            }
            case OP_JMF: {
                Value value = POP();
                if (!CHECK_BOOL(value)) goto ERROR;
                if (!BOOL_TRUE(value)) {
                    ip = vm->code + READ_SHORT();
                } else {
                    ip += 2;
                }
                break;
            }
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
            case OP_INC: {
                Value value = POP();
                if (!CHECK_NUM(value)) goto ERROR;
                PUSH(create_number(value.d_value + READ_BYTE()));
                break;
            }
            case OP_INC_1: {
                Value value = POP();
                if (!CHECK_NUM(value)) goto ERROR;
                PUSH(create_number(value.d_value + 1));
                break;
            }
            case OP_DEC: {
                Value value = POP();
                if (!CHECK_NUM(value)) goto ERROR;
                PUSH(create_number(value.d_value - READ_BYTE()));
                break;
            }
            case OP_NOP:
                break;
            default:
                printf("Unknown instruction %d\n", instruction);
                return INTERPRET_RUNTIME_ERROR;
        }
    }

    ERROR:
    return INTERPRET_RUNTIME_ERROR;

#undef COND_JUMP
#undef BOOL_OP
#undef BINARY_OP
#undef PEEK
#undef PUSH
#undef POP
#undef READ_VAR
#undef READ_CONST
#undef READ_CONST_W
#undef READ_SHORT
#undef READ_BYTE
}
