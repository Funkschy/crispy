// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "vm.h"
#include "memory.h"
#include "debug.h"
#include "opcode.h"
#include "value.h"
#include "../compiler/compiler.h"
#include "../compiler/scanner.h"
#include "dictionary.h"
#include "hashtable.h"
#include "list.h"

static InterpretResult run(Vm *vm);

void frames_init(FrameArray *frames) {
    frames->count = 0;
    frames->cap = 0;
    frames->frame_pointers = NULL;
}

void frames_free(FrameArray *frames) {
    // initial call frame
    call_frame_free(frames->frame_pointers[0]);
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
        frame_arr->frame_pointers = GROW_ARR(frame_arr->frame_pointers, CallFrame *, frame_arr->cap);
    }

    if (index >= frame_arr->count) {
        ++frame_arr->count;
    }

    frame_arr->frame_pointers[index] = frame;
}

void vm_init(Vm *vm, bool interactive) {
    vm->sp = vm->stack;
    vm->first_object = NULL;
    vm->allocated_mem = 0;
    vm->max_alloc_mem = INITIAL_GC_THRESHOLD;
    vm->frame_count = 0;
    vm->interactive = interactive;
    vm->err_flag = false;

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
            call_frame_free(lambda->call_frame);
            free(lambda);
            // TODO size of callframe?
            return sizeof(ObjLambda);
        }
        case OBJ_NATIVE_FUNC: {
            ObjNativeFunc *n_fn = (ObjNativeFunc *) object;
            free(n_fn);
            return sizeof(ObjNativeFunc);
        }
        case OBJ_LIST: {
            ObjList *list = (ObjList *) object;
            val_arr_free(&list->content);
            free(list);
            break;
        }
        case OBJ_DICT: {
            ObjDict *dict = (ObjDict *) object;
            ht_free(&dict->content);
            free(dict);
            break;
        }
    }

    return 0;
}

void vm_free(Vm *vm) {
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

uint32_t add_constant(Vm *vm, CrispyValue value) {
    CallFrame *call_frame = CURR_FRAME(vm);
    if (call_frame->constants.count >= UINT16_MAX) {
        fprintf(stderr, "Too many constants.\n");
        exit(42);
    }

    write_value(&call_frame->constants, value);
    return call_frame->constants.count - 1;
}

static void init_compiler(Compiler *compiler, const char *source) {
    Scanner scanner;
    init_scanner(&scanner, source);

    compiler->scanner = scanner;
    Token err = {TOKEN_ERROR, NULL, 0, 1};
    compiler->previous = err;
    compiler->token = scan_token(&compiler->scanner);
    compiler->next = scan_token(&compiler->scanner);

    VarHashTable ht;
    var_ht_init(&ht, 16);
    compiler->scope[0] = ht;

    compiler->scope_depth = 0;
    compiler->vars_in_scope = 0;
    compiler->print_expr = false;
}

static void free_compiler(Compiler *compiler) {
    var_ht_free(&compiler->scope[0]);
}

static void print_callframe(CallFrame *call_frame) {
    // TODO safe line number information somewhere and printf filename + linenumber
    printf("%p\n", (void *) call_frame->ip);
    printf("Callframe\n");
}

static void panic(Vm *vm, const char *reason) {
    fprintf(stderr, "%s\n", reason);

    CallFrame *current;
    while (vm->frame_count > 0) {
        current = POP_FRAME(vm);
        print_callframe(current);
    }

    vm_free(vm);
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

        code_buff_free(&CURR_FRAME(vm)->code_buffer);
        CodeBuffer code_buffer;
        code_buff_init(&code_buffer);
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
    register CrispyValue *sp = vm->sp;
    register uint8_t *code = curr_frame->code_buffer.code;

    CrispyValue *const_values = curr_frame->constants.values;
    ValueArray *variables = &curr_frame->variables;

#define READ_BYTE() (ip += 1, ip[-1])
#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
#define READ_CONST() (const_values[READ_BYTE()])
#define READ_CONST_W() (ip += 2, const_values[(ip[-2] << 8) | ip[-1]])
#define READ_VAR() (variables->values[READ_BYTE()])
#define POP() (*(sp -= 1))
#define PUSH(value) (*sp = (value), sp += 1)
#define PEEK() (*(sp - 1))
#define BINARY_OP(op)                                           \
    do {                                                        \
        CrispyValue second = POP();                             \
        CrispyValue first = POP();                              \
        if (!CHECK_NUM(first) || !CHECK_NUM(second))            \
            goto ERROR;                                         \
        first.d_value = first.d_value op second.d_value;        \
        PUSH(first);                                            \
    } while (false)

#define COND_JUMP(op)                                           \
    do {                                                        \
        CrispyValue second = POP();                             \
        CrispyValue first = POP();                              \
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
                CrispyValue zero = create_number(0.0);
                PUSH(zero);
                break;
            }
            case OP_LDC_1: {
                CrispyValue one = create_number(1.0);
                PUSH(one);
                break;
            }
            case OP_CALL: {
                uint8_t num_args = READ_BYTE();
                CrispyValue *pos = (sp - num_args - 1);

                if (pos->type != OBJECT) {
                    fprintf(stderr, "Trying to call primitive CrispyValue\n");
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
                    CrispyValue args[num_args];
                    CrispyValue *start = sp - num_args;

                    for (int i = 0; i < num_args; ++i) {
                        args[i] = *(start++);
                        --sp;
                    }

                    CrispyValue res;
                    if (n_fn->system_func) {
                        vm->sp = sp;
                        res = ((CrispyValue (*)(CrispyValue *, Vm *vm)) n_fn->func_ptr)(args, vm);

                        if (vm->err_flag) {
                            vm->err_flag = false;
                            // the system native functions return the error message as a crispy string
                            print_value(res, true, false);
                            goto ERROR;
                        }

                    } else {
                        res = ((CrispyValue (*)(CrispyValue *)) n_fn->func_ptr)(args);
                    }

                    // pop
                    --sp;

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
                CrispyValue args[num_args];
                for (int i = 0; i < num_args; ++i) {
                    args[i] = POP();
                }

                CallFrame *call_frame = new_temp_call_frame(lambda->call_frame);

                PUSH_FRAME(vm, call_frame);
                CrispyValue *before_sp = sp;
                for (int i = 0; i < num_args; ++i) {
                    PUSH(args[i]);
                }
                vm->sp = sp;
                InterpretResult result = run(vm);

                RM_FRAME(vm);
                temp_call_frame_free(call_frame);

                if (result != INTERPRET_OK) {
                    return result;
                }

                sp = before_sp;
                break;
            }
            case OP_ADD: {
                CrispyValue second = POP();
                CrispyValue first = POP();

                if (first.type == NUMBER && second.type == NUMBER) {
                    first.d_value += second.d_value;
                    PUSH(first);
                    break;
                }

                if (first.type == OBJECT) {
                    Object *first_obj = first.o_value;

                    switch (first_obj->type) {
                        case OBJ_STRING: {
                            if (second.type != OBJECT || second.o_value->type != OBJ_STRING) {
                                fprintf(stderr,
                                        "Only strings can be appended to strings. Consider using the 'str' function\n");
                                goto ERROR;
                            }
                            ObjString *first_str = (ObjString *) first_obj;
                            ObjString *second_str = (ObjString *) second.o_value;

                            ObjString *dest = new_empty_string(vm, (first_str->length + second_str->length));
                            memcpy((char *) dest->start, first_str->start, first_str->length);
                            memcpy((char *) (dest->start + first_str->length), second_str->start, second_str->length);

                            PUSH(create_object((Object *) dest));
                            break;
                        }
                        case OBJ_LIST: {
                            ObjList *clone = clone_list(vm, (ObjList *) first_obj);
                            list_append(clone, second);

                            PUSH(create_object((Object *) clone));
                            break;
                        }
                        default:
                            fprintf(stderr, "Invalid target for addition\n");
                            goto ERROR;
                    }
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
                CrispyValue second = POP();
                CrispyValue first = POP();

                if (first.type != NUMBER || second.type != NUMBER) {
                    fprintf(stderr, "Modulo operator (%%) only works on numbers\n");
                    goto ERROR;
                }

                int64_t first_int = (int64_t) first.d_value;
                int64_t second_int = (int64_t) second.d_value;

                PUSH(create_number(first_int % second_int));
                break;
            }
            case OP_DIV: {
                CrispyValue second = POP();
                CrispyValue first = POP();
                if (!CHECK_NUM(first) || !CHECK_NUM(second)) {
                    goto ERROR;
                }

                if (second.d_value == 0) {
                    panic(vm, "Cannot divide by zero\n");
                }

                first.d_value = first.d_value / second.d_value;
                PUSH(first);
                break;
            }
            case OP_POW: {
                CrispyValue exponent = POP();
                CrispyValue base = POP();

                if (!CHECK_NUM(base) || !CHECK_NUM(exponent)) {
                    goto ERROR;
                }

                PUSH(create_number(pow(base.d_value, exponent.d_value)));

                break;
            }
            case OP_OR: {
                CrispyValue second = POP();
                CrispyValue first = POP();

                if (first.type != BOOLEAN || second.type != BOOLEAN) {
                    goto ERROR;
                }

                PUSH(create_bool(first.p_value || second.p_value));

                break;
            }
            case OP_AND: {
                CrispyValue second = POP();
                CrispyValue first = POP();

                if (first.type != BOOLEAN || second.type != BOOLEAN) {
                    goto ERROR;
                }

                PUSH(create_bool(first.p_value && second.p_value));

                break;
            }
            case OP_EQUAL: {
                CrispyValue second = POP();
                CrispyValue first = POP();
                PUSH(create_bool(cmp_values(first, second) == 0));
                break;
            }
            case OP_NOT_EQUAL: {
                CrispyValue second = POP();
                CrispyValue first = POP();
                PUSH(create_bool(cmp_values(first, second) != 0));
                break;
            }
            case OP_GE: {
                // TODO exception for non orderable type (e.g nil)
                CrispyValue second = POP();
                CrispyValue first = POP();
                PUSH(create_bool(cmp_values(first, second) >= 0));
                break;
            }
            case OP_LE: {
                CrispyValue second = POP();
                CrispyValue first = POP();
                PUSH(create_bool(cmp_values(first, second) <= 0));
                break;
            }
            case OP_GT: {
                CrispyValue second = POP();
                CrispyValue first = POP();
                PUSH(create_bool(cmp_values(first, second) > 0));
                break;
            }
            case OP_LT: {
                CrispyValue second = POP();
                CrispyValue first = POP();
                PUSH(create_bool(cmp_values(first, second) < 0));
                break;
            }
            case OP_NEGATE: {
                CrispyValue val = create_number(POP().d_value * -1);
                PUSH(val);
                break;
            }
            case OP_LOAD: {
                CrispyValue val = READ_VAR();
                PUSH(val);
                break;
            }
            case OP_LOAD_OFFSET: {
                uint8_t scope = READ_BYTE();
                uint8_t index = READ_BYTE();

                CallFrame *frame = FRAME_AT(vm, scope);
                CrispyValue val = frame->variables.values[index];

                PUSH(val);

                break;
            }
            case OP_STORE_OFFSET: {
                uint8_t scope = READ_BYTE();
                uint8_t index = READ_BYTE();

                CallFrame *frame = FRAME_AT(vm, scope);
                CrispyValue val = POP();

                frame->variables.values[index] = val;
                break;
            }
            case OP_STORE: {
                uint8_t index = READ_BYTE();
                CrispyValue val = POP();
                write_at(variables, index, val);
                break;
            }
            case OP_POP:
                --sp;
                break;
            case OP_DUP: {
                CrispyValue val = PEEK();
                PUSH(val);
                break;
            }
            case OP_JMP:
                ip = code + READ_SHORT();
                break;
            case OP_JMT: {
                CrispyValue value = POP();
                if (!CHECK_BOOL(value)) { goto ERROR; }
                if (BOOL_TRUE(value)) {
                    ip = code + READ_SHORT();
                } else {
                    ip += 2;
                }
                break;
            }
            case OP_JMF: {
                CrispyValue value = POP();
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
                CrispyValue value = POP();

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
                CrispyValue value = POP();
                printf("> ");
                print_value(value, true, true);
                break;
            }
            case OP_DICT_NEW: {
                HashTable content;
                ht_init(&content, HT_KEY_OBJSTRING, 8, free_objstring);

                ObjDict *dict = new_dict(vm, content);
                CrispyValue value = create_object((Object *) dict);
                PUSH(value);
                break;
            }
            case OP_LIST_NEW: {
                ObjList *list = new_list(vm, 0);
                CrispyValue list_val = create_object((Object *) list);

                PUSH(list_val);
                break;
            }
            case OP_LIST_APPEND: {
                CrispyValue value = POP();
                CrispyValue list_val = PEEK();

                if (list_val.type != OBJECT || list_val.o_value->type != OBJ_LIST) {
                    fprintf(stderr, "Can only put values into lists\n");
                    goto ERROR;
                }

                ObjList *list = (ObjList *) list_val.o_value;

                list_append(list, value);
                break;
            }
            case OP_STRUCT_SET: {
                CrispyValue value = POP();
                CrispyValue key = POP();
                CrispyValue structure = PEEK();

                if (structure.type != OBJECT) {
                    fprintf(stderr, "Trying to retrieve an element from a primitive value\n");
                    goto ERROR;
                }

                Object *obj = structure.o_value;

                switch (obj->type) {
                    case OBJ_DICT: {
                        ObjDict *dict = (ObjDict *) obj;

                        if (key.type != OBJECT) {
                            fprintf(stderr, "Only strings can be used as indices for dictionaries\n");
                            goto ERROR;
                        }

                        HTItemKey ht_key;
                        Object *key_obj = key.o_value;

                        if (key_obj->type != OBJ_STRING) {
                            fprintf(stderr, "Only strings can be used as indices for dictionaries\n");
                            goto ERROR;
                        }

                        ht_key.key_obj_string = (ObjString *) key_obj;
                        ht_put(&dict->content, ht_key, value);
                        break;
                    }
                    case OBJ_LIST: {
                        ObjList *list = (ObjList *) obj;

                        // Check if key is an integer
                        if (key.type != NUMBER || floor(key.d_value) != key.d_value) {
                            fprintf(stderr, "Only integers can be used as indices for lists\n");
                            goto ERROR;
                        }

                        int64_t index = (int64_t) key.d_value;
                        bool success = list_add(list, index, value);

                        if (!success) {
                            fprintf(stderr, "Index out of bounds\n");
                            goto ERROR;
                        }

                        break;
                    }
                    default:
                        fprintf(stderr, "Invalid receiver for set operation\n");
                        goto ERROR;
                }

                break;
            }
            case OP_STRUCT_GET: {
                CrispyValue key_val = POP();
                CrispyValue struct_val = POP();

                if (struct_val.type != OBJECT) {
                    fprintf(stderr, "Trying to retrieve an element from a primitive value\n");
                    goto ERROR;
                }

                Object *obj = struct_val.o_value;

                switch (obj->type) {
                    case OBJ_DICT: {
                        if (key_val.type != OBJECT) {
                            fprintf(stderr, "Only strings can be used as indices for dictionaries\n");
                            goto ERROR;
                        }


                        ObjDict *dict = (ObjDict *) obj;
                        Object *key_obj = key_val.o_value;

                        if (key_obj->type != OBJ_STRING) {
                            fprintf(stderr, "Only strings can be used as indices for dictionaries\n");
                            goto ERROR;
                        }

                        ObjString *key_string = (ObjString *) key_obj;

                        HTItemKey key;
                        key.key_obj_string = key_string;

                        CrispyValue result = ht_get(&dict->content, key);
                        PUSH(result);
                        break;
                    }
                    case OBJ_LIST: {
                        ObjList *list = (ObjList *) obj;

                        // Check if key is an integer
                        if (key_val.type != NUMBER || floor(key_val.d_value) != key_val.d_value) {
                            fprintf(stderr, "Only integers can be used as indices for lists\n");
                            goto ERROR;
                        }

                        int64_t index = (int64_t) key_val.d_value;
                        CrispyValue value;
                        bool success = list_get(list, index, &value);

                        if (!success) {
                            fprintf(stderr, "Index out of bounds\n");
                            goto ERROR;
                        }

                        PUSH(value);
                        break;
                    }
                    default:
                        fprintf(stderr, "Invalid receiver for get operation\n");
                        goto ERROR;
                }

                break;
            }
            case OP_STRUCT_PEEK: {
                CrispyValue key_val = PEEK();
                CrispyValue struct_val = sp[-2];

                if (struct_val.type != OBJECT) {
                    fprintf(stderr, "Trying to retrieve an element from a primitive value\n");
                    goto ERROR;
                }

                Object *obj = struct_val.o_value;

                switch (obj->type) {
                    case OBJ_LIST: {
                        ObjList *list = (ObjList *) struct_val.o_value;

                        // Check if key is an integer
                        if (key_val.type != NUMBER || floor(key_val.d_value) != key_val.d_value) {
                            fprintf(stderr, "Only integers can be used as indices for lists\n");
                            goto ERROR;
                        }

                        uint32_t index = (uint32_t) key_val.d_value;
                        CrispyValue value;
                        bool success = list_get(list, index, &value);

                        if (!success) {
                            fprintf(stderr, "Index out of bounds\n");
                            goto ERROR;
                        }

                        PUSH(value);
                        break;
                    }
                    case OBJ_DICT: {
                        ObjDict *dict = (ObjDict *) struct_val.o_value;
                        ObjString *key_string = (ObjString *) key_val.o_value;

                        HTItemKey key;
                        key.key_obj_string = key_string;

                        CrispyValue result = ht_get(&dict->content, key);
                        PUSH(result);
                        break;
                    }
                    default:
                        fprintf(stderr, "Invalid receiver for get operation\n");
                        goto ERROR;
                }

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
