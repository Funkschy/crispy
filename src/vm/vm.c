// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "memory.h"
#include "debug.h"
#include "opcode.h"
#include "value.h"
#include "../compiler/compiler.h"
#include "../compiler/scanner.h"
#include "dictionary.h"
#include "hashtable.h"

static InterpretResult run(Vm *vm);

void frames_init(FrameArray *frames) {
    frames->count = 0;
    frames->cap = 0;
    frames->frame_pointers = NULL;
}

void frames_free(FrameArray *frames) {
    // initial call frame
    free_call_frame(frames->frame_pointers[0]);
    FREE_ARR(frames->frame_pointers);
    frames_init(frames);
}

void frames_write_at(FrameArray *frame_arr, uint32_t index, CallFrame *frame) {
    if (index > frame_arr->cap) {
        fprintf(stderr, "Invalid write in frame array");
        exit(100);
    }

    if (index == frame_arr->cap) {
        frame_arr->cap = GROW_CAP(frame_arr->cap);
        frame_arr->frame_pointers = GROW_ARR(frame_arr->frame_pointers, Value, frame_arr->cap);
    }

    if (index >= frame_arr->count) {
        ++frame_arr->count;
    }

    frame_arr->frame_pointers[index] = frame;
}

void init_vm(Vm *vm, bool interactive) {
    vm->sp = vm->stack;
    vm->first_object = NULL;
    vm->allocated_mem = 0;
    vm->max_alloc_mem = INITIAL_GC_THRESHOLD;
    vm->frame_count = 0;
    vm->interactive = interactive;

    FrameArray frames;
    frames_init(&frames);
    vm->frames = frames;

    CallFrame *call_frame = new_call_frame();
    frames_write_at(&vm->frames, vm->frame_count++, call_frame);

    HashTable strings;
    ht_init(&strings, HT_KEY_IDENT_STRING, 16, free_string_literal);
    vm->strings = strings;
}

size_t free_object(Object *object) {
    switch (object->type) {
        case OBJ_STRING: {
            ObjString *string = (ObjString *) object;
            size_t length = string->length;

            free((void *) string->start);
            free(object);
            return sizeof(ObjString) + length * sizeof(char);
        }
        case OBJ_LAMBDA: {
            ObjLambda *lambda = (ObjLambda *) object;
            free_call_frame(lambda->call_frame);
            free(lambda);
            // TODO size of callframe?
            return sizeof(ObjLambda);
        }
        case OBJ_NATIVE_FUNC: {
            ObjNativeFunc *n_fn = (ObjNativeFunc *) object;
            free(n_fn);
            return sizeof(ObjNativeFunc);
        }
        case OBJ_LIST:
            printf("Lists can't be freed yet\n");
            break;
        case OBJ_DICT: {
            ObjDict *dict = (ObjDict *) object;
            ht_free(&dict->content);
            free(dict);
            break;
        }
    }

    return 0;
}

void free_vm(Vm *vm) {
    frames_free(&vm->frames);

    Object **obj = &vm->first_object;

    while (*obj) {
        Object *to_del = *obj;
        *obj = to_del->next;
        free_object(to_del);
    }

    vm->sp = NULL;
    vm->first_object = NULL;
    vm->allocated_mem = 0;
    vm->max_alloc_mem = 0;

    ht_free(&vm->strings);
}

void write_code_buffer(CodeBuffer *code_buffer, uint8_t instruction) {
    if (code_buffer->count >= code_buffer->cap) {
        code_buffer->cap = GROW_CAP(code_buffer->cap);
        code_buffer->code = GROW_ARR(code_buffer->code, uint8_t, code_buffer->cap);
    }

    code_buffer->code[code_buffer->count++] = instruction;
}

uint32_t add_constant(Vm *vm, Value value) {
    write_value(&CURR_FRAME(vm)->constants, value);
    return CURR_FRAME(vm)->constants.count - 1;
}

static void init_compiler(Compiler *compiler, const char *source) {
    Scanner scanner;
    init_scanner(&scanner, source);

    compiler->scanner = scanner;
    Token err = {TOKEN_ERROR, NULL, 0, 1};
    compiler->previous = err;
    compiler->token = scan_token(&compiler->scanner);
    compiler->next = scan_token(&compiler->scanner);

    VariableArray variables;
    init_variable_array(&variables);
    compiler->scope[0] = variables;
    compiler->scope_depth = 0;
    compiler->vars_in_scope = 0;
    compiler->print_expr = false;

    HashTable ht;
    // TODO change to ident string
    ht_init(&ht, HT_KEY_CSTRING, 16, free_string_literal);
    compiler->natives = ht;
}

static void free_compiler(Compiler *compiler) {
    free_variable_array(&compiler->scope[0]);
    ht_free(&compiler->natives);
}

static void print_callframe(CallFrame *call_frame) {
    // TODO safe line number information somewhere and printf filename + linenumber
    printf("Callframe\n");
}

static void panic(Vm *vm, const char *reason) {
    fprintf(stderr, "%s\n", reason);

    CallFrame *current = POP_FRAME(vm);
    while (current != NULL) {
        print_callframe(current);
        current = POP_FRAME(vm);
    }

    free_vm(vm);
    exit(42);
}

InterpretResult interpret(Vm *vm, const char *source) {
    Compiler compiler;
    init_compiler(&compiler, source);

    vm->compiler = compiler;
    int compile_result = compile(vm);

    if (compile_result) {
        return INTERPRET_COMPILE_ERROR;
    }

#if DEBUG_SHOW_DISASSEMBLY
    disassemble_curr_frame(vm, "Main Program");
#endif

    CURR_FRAME(vm)->ip = CURR_FRAME(vm)->code_buffer.code;
    InterpretResult result = run(vm);
    free_compiler(&vm->compiler);

    return result;
}

InterpretResult interpret_interactive(Vm *vm, const char *source) {
    static bool first_time = true;

    if (first_time) {
        first_time = false;
        Compiler compiler;
        init_compiler(&compiler, source);
        vm->compiler = compiler;
    } else {
        init_scanner(&vm->compiler.scanner, source);
        vm->compiler.scanner = vm->compiler.scanner;
        vm->compiler.token = scan_token(&vm->compiler.scanner);
        vm->compiler.next = scan_token(&vm->compiler.scanner);

        free_code_buffer(&CURR_FRAME(vm)->code_buffer);
        CodeBuffer code_buffer;
        init_code_buffer(&code_buffer);
        CURR_FRAME(vm)->code_buffer = code_buffer;
    }

    int compile_result = compile(vm);

    if (compile_result) {
        return INTERPRET_COMPILE_ERROR;
    }

#if DEBUG_SHOW_DISASSEMBLY
    disassemble_curr_frame(vm, "Last input");
#endif

    CURR_FRAME(vm)->ip = CURR_FRAME(vm)->code_buffer.code;
    InterpretResult result = run(vm);

    return result;
}

static InterpretResult run(Vm *vm) {
    CallFrame *curr_frame = CURR_FRAME(vm);

    register uint8_t *ip = curr_frame->ip;
    register Value *sp = vm->sp;
    register uint8_t *code = curr_frame->code_buffer.code;

    Value *const_values = curr_frame->constants.values;
    ValueArray *variables = &curr_frame->variables;

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
        first.d_value = first.d_value op second.d_value;        \
        PUSH(first);                                            \
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
        {
            printf("-----\n");
            long stack_size = sp - vm->stack;
            for (int i = 0; i < stack_size; ++i) {
                printf("[%d] ", i);
                print_value(vm->stack[i], false, true);
                print_type(vm->stack[i]);
            }
            printf("sp: %li\n", stack_size);
            printf("ip: %li\n", ip - code);
            disassemble_instruction(vm, (int) (ip - code));
            printf("-----\n");
        }
#endif

#if DEBUG_TYPE_CHECK
        {
            long stack_size = sp - vm->stack;
            if (stack_size < 0) {
                printf("Negative stack pointer\n");
            }
        }
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
            case OP_CALL: {
                uint8_t num_args = READ_BYTE();
                Value *pos = (sp - num_args - 1);

                if (pos->type != OBJECT) {
                    fprintf(stderr, "Trying to call primitive Value\n");
                    goto ERROR;
                }

                Object *object = pos->o_value;

                if (object->type == OBJ_NATIVE_FUNC) {
                    if (num_args != 1) {
                        fprintf(stderr, "Native functions may only receive one argument\n");
                        goto ERROR;
                    }

                    ObjNativeFunc *n_fn = (ObjNativeFunc *) object;

                    uint8_t expected = n_fn->num_params;
                    if (expected != num_args) {
                        fprintf(stderr, "Invalid number of arguments. Expected %d, but got %d\n", expected, num_args);
                        goto ERROR;
                    }

                    // Pop arguments from stack
                    Value args[num_args];
                    Value *start = sp - num_args;

                    for (int i = 0; i < num_args; ++i) {
                        args[i] = *(start++);
                        --sp;
                    }

                    Value res;
                    if (n_fn->system_func) {
                        vm->sp = sp;
                        res = ((Value (*)(Value *, Vm *vm)) n_fn->func_ptr)(args, vm);
                    } else {
                        res = ((Value (*)(Value *)) n_fn->func_ptr)(args);
                    }

                    POP();

                    PUSH(res);
                    break;
                }

                if (object->type != OBJ_LAMBDA) {
                    fprintf(stderr, "Trying to call non callable Object\n");
                    goto ERROR;
                }
                ObjLambda *lambda = ((ObjLambda *) object);

                uint8_t expected = lambda->num_params;
                if (expected != num_args) {
                    fprintf(stderr, "Invalid number of arguments. Expected %d, but got %d\n", expected, num_args);
                    goto ERROR;
                }

                // Pop arguments from stack
                Value args[num_args];
                for (int i = 0; i < num_args; ++i) {
                    args[i] = POP();
                }

                CallFrame *call_frame = new_temp_call_frame(lambda->call_frame);

                PUSH_FRAME(vm, call_frame);
                Value *before_sp = sp;
                for (int i = 0; i < num_args; ++i) {
                    PUSH(args[i]);
                }
                vm->sp = sp;
                InterpretResult result = run(vm);

                if (result != INTERPRET_OK) {
                    return result;
                }

                POP_FRAME(vm);

                free_temp_call_frame(call_frame);

                sp = before_sp;
                break;
            }
            case OP_ADD: {
                Value second = POP();
                Value first = POP();

                if (first.type == NUMBER && second.type == NUMBER) {
                    first.d_value += second.d_value;
                    PUSH(first);
                    break;
                }

                // TODO handle lists
                if (first.type == OBJECT && second.type == OBJECT) {
                    Object *first_obj = first.o_value;
                    Object *second_obj = second.o_value;

                    if (first_obj->type != OBJ_STRING || second_obj->type != OBJ_STRING) {
                        goto ERROR;
                    }

                    ObjString *first_str = (ObjString *) first_obj;
                    ObjString *second_str = (ObjString *) second_obj;

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
            case OP_MOD: {
                Value second = POP();
                Value first = POP();

                if (first.type != NUMBER || second.type != NUMBER) {
                    fprintf(stderr, "Modulo operator (%%) only works on numbers");
                    goto ERROR;
                }

                int64_t first_int = (int64_t) first.d_value;
                int64_t second_int = (int64_t) second.d_value;

                PUSH(create_number(first_int % second_int));
                break;
            }
            case OP_DIV: {
                Value second = POP();
                Value first = POP();
                if (!CHECK_NUM(first) || !CHECK_NUM(second)) {
                    goto ERROR;
                }

                if (second.d_value == 0) {
                    panic(vm, "Cannot divide by zero");
                }

                first.d_value = first.d_value / second.d_value;
                PUSH(first);
                break;
            }
            case OP_OR: {
                Value second = POP();
                Value first = POP();

                if (first.type != BOOLEAN || second.type != BOOLEAN) {
                    goto ERROR;
                }

                PUSH(create_bool(first.p_value || second.p_value));

                break;
            }
            case OP_AND: {
                Value second = POP();
                Value first = POP();

                if (first.type != BOOLEAN || second.type != BOOLEAN) {
                    goto ERROR;
                }

                PUSH(create_bool(first.p_value && second.p_value));

                break;
            }
            case OP_EQUAL: {
                Value second = POP();
                Value first = POP();
                PUSH(create_bool(cmp_values(first, second) == 0));
                break;
            }
            case OP_NOT_EQUAL: {
                Value second = POP();
                Value first = POP();
                PUSH(create_bool(cmp_values(first, second) != 0));
                break;
            }
            case OP_GE: {
                // TODO exception for non orderable type (e.g nil)
                Value second = POP();
                Value first = POP();
                PUSH(create_bool(cmp_values(first, second) >= 0));
                break;
            }
            case OP_LE: {
                Value second = POP();
                Value first = POP();
                PUSH(create_bool(cmp_values(first, second) <= 0));
                break;
            }
            case OP_GT: {
                Value second = POP();
                Value first = POP();
                PUSH(create_bool(cmp_values(first, second) > 0));
                break;
            }
            case OP_LT: {
                Value second = POP();
                Value first = POP();
                PUSH(create_bool(cmp_values(first, second) < 0));
                break;
            }
            case OP_NEGATE: {
                Value val = create_number(POP().d_value * -1);
                PUSH(val);
                break;
            }
            case OP_LOAD: {
                Value val = READ_VAR();
                PUSH(val);
                break;
            }
            case OP_LOAD_OFFSET: {
                uint8_t scope = READ_BYTE();
                uint8_t index = READ_BYTE();

                CallFrame *frame = FRAME_AT(vm, scope);
                Value val = frame->variables.values[index];

                PUSH(val);

                break;
            }
            case OP_STORE_OFFSET: {
                uint8_t scope = READ_BYTE();
                uint8_t index = READ_BYTE();

                CallFrame *frame = FRAME_AT(vm, scope);
                Value val = POP();

                frame->variables.values[index] = val;
                break;
            }
            case OP_STORE: {
                uint8_t index = READ_BYTE();
                Value val = POP();
                write_at(variables, index, val);
                break;
            }
            case OP_POP:
                POP();
                break;
            case OP_DUP: {
                Value val = PEEK();
                PUSH(val);
                break;
            }
            case OP_JMP:
                ip = code + READ_SHORT();
                break;
            case OP_JMT: {
                Value value = POP();
                if (!CHECK_BOOL(value)) { goto ERROR; }
                if (BOOL_TRUE(value)) {
                    ip = code + READ_SHORT();
                } else {
                    ip += 2;
                }
                break;
            }
            case OP_JMF: {
                Value value = POP();
                if (!CHECK_BOOL(value)) { goto ERROR; }
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
            case OP_INC_1: {
                uint8_t index = READ_BYTE();
                ++variables->values[index].d_value;
                break;
            }
            case OP_DEC_1: {
                uint8_t index = READ_BYTE();
                --variables->values[index].d_value;
                break;
            }
            case OP_NOT: {
                Value value = POP();

                if (value.type != BOOLEAN) {
                    goto ERROR;
                }

                value.p_value = value.p_value == 0 ? 1 : 0;
                PUSH(value);
                break;
            }
            case OP_TRUE:
                PUSH(create_bool(true));
                break;
            case OP_FALSE:
                PUSH(create_bool(false));
                break;
            case OP_NOP:
                PUSH(create_bool(false));
                break;
            case OP_NIL:
                PUSH(create_nil());
                break;
            case OP_PRINT: {
                Value value = POP();
                printf("> ");
                print_value(value, true, true);
                break;
            }
            case OP_DICT_NEW: {
                HashTable content;
                ht_init(&content, HT_KEY_OBJSTRING, 8, free_objstring);

                ObjDict *dict = new_dict(vm, content);
                Value value = create_object((Object *) dict);
                PUSH(value);
                break;
            }
            case OP_DICT_PUT: {
                Value value = POP();
                Value key = POP();

                Value dict_val = PEEK();
                ObjDict *dict = (ObjDict *) dict_val.o_value;

                HTItemKey ht_key;
                ht_key.key_obj_string = (ObjString *) key.o_value;
                ht_put(&dict->content, ht_key, value);

                break;
            }
            case OP_DICT_GET: {
                Value key_val = POP();
                Value dict_val = POP();

                ObjDict *dict = (ObjDict *) dict_val.o_value;
                ObjString *key_string = (ObjString *) key_val.o_value;

                HTItemKey key;
                key.key_obj_string = key_string;

                Value result = ht_get(&dict->content, key);
                PUSH(result);

                break;
            }
            default:
                printf("Unknown instruction %d\n", instruction);
                return INTERPRET_RUNTIME_ERROR;
        }
    }

    ERROR:
    return INTERPRET_RUNTIME_ERROR;

#undef COND_JUMP
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
