#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "memory.h"
#include "debug.h"
#include "opcode.h"
#include "value.h"
#include "../compiler/compiler.h"

static InterpretResult run(Vm *vm);

void free_code_buffer(CodeBuffer *code_buffer) {
    FREE_ARR(code_buffer->code);
    code_buffer->cap = 0;
    code_buffer->count = 0;
    code_buffer->code = NULL;

    free_value_array(&code_buffer->constants);
    free_value_array(&code_buffer->variables);
}

void push_call_frame(Vm *vm) {
    CallFrame call_frame;
    init_call_frame(&call_frame);

    vm->frames[vm->frame_count++] = call_frame;
}

void pop_call_frame(Vm *vm) {
    --vm->frame_count;
}

void init_vm(Vm *vm) {
    vm->sp = vm->stack;
    vm->first_object = NULL;
    vm->num_objects = 0;
    vm->max_objects = INITIAL_GC_THRESHOLD;
    vm->frame_count = 0;

    push_call_frame(vm);
}

void free_object(Object *object) {
    switch (object->type) {
        case OBJ_STRING: {
            ObjString *string = (ObjString *) object;
            free((void *) string->start);
            free(object);
            break;
        }
        case OBJ_LAMBDA: {
            ObjLambda *lambda = (ObjLambda *) object;
            free_call_frame(&lambda->call_frame);
            free(lambda);
        }
        default:
            break;
    }
}

void free_vm(Vm *vm) {
    Object **obj = &vm->first_object;

    while (*obj) {
        Object *to_del = *obj;
        *obj = to_del->next;
        free_object(to_del);
    }

    vm->sp = NULL;
    vm->first_object = NULL;
    vm->num_objects = 0;
    vm->max_objects = 0;

    free_code_buffer(&CURR_FRAME(vm).code_buffer);

    while (vm->frame_count > 0) {
        pop_call_frame(vm);
    }
}

void write_code_buffer(CodeBuffer *code_buffer, uint8_t instruction) {
    if (code_buffer->count >= code_buffer->cap) {
        code_buffer->cap = GROW_CAP(code_buffer->cap);
        code_buffer->code = GROW_ARR(code_buffer->code, uint8_t, code_buffer->cap);
    }

    code_buffer->code[code_buffer->count++] = instruction;
}

uint32_t add_constant(CodeBuffer *code_buffer, Value value) {
    write_value(&code_buffer->constants, value);
    return code_buffer->constants.count - 1;
}

static void init_compiler(Compiler *compiler, const char *source) {
    Scanner scanner;
    init_scanner(&scanner, source);

    compiler->scanner = scanner;
    compiler->token = scan_token(&compiler->scanner);

    VariableArray variables;
    init_variable_array(&variables);
    compiler->scope[0] = variables;
    compiler->scope_depth = 0;
    compiler->vars_in_scope = 0;
}

static void free_compiler(Compiler *compiler) {
    free_variable_array(&compiler->scope[0]);
}

InterpretResult interpret(Vm *vm, const char *source) {
    Compiler compiler;
    init_compiler(&compiler, source);

    vm->compiler = compiler;
    compile(vm);

#if DEBUG_SHOW_DISASSEMBLY
    disassemble_vm(vm, "Main Program");
#endif

    CURR_FRAME(vm).ip = CURR_FRAME(vm).code_buffer.code;
    InterpretResult result = run(vm);
    free_compiler(&vm->compiler);

    return result;
}

static InterpretResult run(Vm *vm) {
    CallFrame *curr_frame = &CURR_FRAME(vm);

    register uint8_t *ip = curr_frame->ip;
    register Value *sp = vm->sp;
    register uint8_t *code = curr_frame->code_buffer.code;

    Value *const_values = curr_frame->code_buffer.constants.values;
    ValueArray *variables = &curr_frame->code_buffer.variables;

    Value *start_sp = sp;

// Return byte at ip and advance ip
#define READ_BYTE() (*ip++)
#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
#define READ_CONST() (const_values[READ_BYTE()])
#define READ_CONST_W() (const_values[(READ_BYTE() << 8) | READ_BYTE()])
#define READ_VAR() (variables->values[READ_BYTE()])
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

#define COND_JUMP(op)                                           \
    do {                                                        \
        Value second = POP();                                   \
        Value first = POP();                                    \
        if (first.p_value op second.p_value) {                  \
            ip = code + READ_SHORT();                           \
        } else {                                                \
            READ_SHORT();                                       \
        }                                                       \
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
        printf("ip: %li\n", ip - code);
        disassemble_instruction(vm, (int) (ip - code));
#endif

        switch (instruction = (OP_CODE) READ_BYTE()) {
            case OP_RETURN:
                *start_sp = POP();
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
            case OP_CALL: {
                uint8_t num_args = READ_BYTE();
                ObjLambda *lambda = ((ObjLambda *) (sp - num_args - 1)->o_value);
                Value args[num_args];
                for (int i = 0; i < num_args; ++i) {
                    args[i] = POP();
                }
                PUSH_FRAME(vm, lambda->call_frame);
                Value *before_sp = sp;
                for (int i = num_args - 1; i >= 0; --i) {
                    PUSH(args[i]);
                }
                vm->sp = sp;
                run(vm);
                lambda->call_frame = POP_FRAME(vm);
                sp = before_sp;
                break;
            }
            case OP_ADD: {
                Value second = POP();
                Value first = POP();

                if (first.type == NUMBER && second.type == NUMBER) {
                    PUSH(create_number(first.d_value + second.d_value));
                    break;
                }

                // TODO handle non string objects
                if (first.type == OBJECT && second.type == OBJECT) {
                    ObjString *first_str = (ObjString *) first.o_value;
                    ObjString *second_str = (ObjString *) second.o_value;

                    ObjString *dest = new_empty_string(vm, (first_str->length + second_str->length));
                    memcpy((char *) dest->start, first_str->start, first_str->length);
                    memcpy((char *) (dest->start + first_str->length), second_str->start, second_str->length);

                    PUSH(create_object((Object *) dest));
                } else {
                    goto ERROR;
                }

                break;
            }
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
                write_at(variables, index, POP());
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
                ip = code + READ_SHORT();
                break;
            case OP_JMT: {
                Value value = POP();
                if (!CHECK_BOOL(value)) goto ERROR;
                if (BOOL_TRUE(value)) {
                    ip = code + READ_SHORT();
                } else {
                    ip += 2;
                }
                break;
            }
            case OP_JMF: {
                Value value = POP();
                if (!CHECK_BOOL(value)) goto ERROR;
                if (!BOOL_TRUE(value)) {
                    ip = code + READ_SHORT();
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
