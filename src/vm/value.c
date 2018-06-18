// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>
#include <string.h>

#include "value.h"
#include "vm.h"
#include "memory.h"
#include "dictionary.h"

void val_arr_init(ValueArray *value_array) {
    value_array->cap = 0;
    value_array->count = 0;
    value_array->values = NULL;
}

void val_arr_free(ValueArray *value_array) {
    FREE_ARR(value_array->values);
    val_arr_init(value_array);
}

void write_value(ValueArray *value_array, CrispyValue value) {
    if (value_array->count >= value_array->cap) {
        value_array->cap = GROW_CAP(value_array->cap);
        value_array->values = GROW_ARR(value_array->values, CrispyValue, value_array->cap);
    }

    value_array->values[value_array->count++] = value;
}

void write_at(ValueArray *value_array, uint32_t index, CrispyValue value) {
    while (index >= value_array->cap) {
        value_array->cap = GROW_CAP(value_array->cap);
        value_array->values = GROW_ARR(value_array->values, CrispyValue, value_array->cap);
    }

    if (index >= value_array->count) {
        ++value_array->count;
    }

    value_array->values[index] = value;
}

size_t value_to_string(CrispyValue value, char **dest) {
    char *string = NULL;
    size_t str_len = 0;

    switch (value.type) {
        case NUMBER: {
            char s[17];
            snprintf(s, 17, "%.15g", value.d_value);
            string = strdup(s);
            str_len = strlen(s);
            break;
        }
        case OBJECT: {
            Object *object = value.o_value;

            switch (object->type) {
                case OBJ_STRING: {
                    ObjString *obj_string = ((ObjString *) object);
                    string = malloc((obj_string->length + 3) * sizeof(char));
                    memcpy(string + 1, obj_string->start, obj_string->length);
                    string[0] = '"';
                    string[obj_string->length + 1] = '"';
                    string[obj_string->length + 2] = '\0';
                    str_len = ((ObjString *) object)->length + 2;
                    break;
                }
                case OBJ_LAMBDA:
                    // TODO arity
                    string = strdup("<function>");
                    str_len = 10;
                    break;
                case OBJ_NATIVE_FUNC:
                    // TODO arity
                    string = strdup("<native function>");
                    str_len = 17;
                    break;
                case OBJ_DICT: {
                    char *dict_string = dict_to_string((ObjDict *) object);
                    string = dict_string;
                    str_len = strlen(dict_string);
                    break;
                }
                case OBJ_LIST:
                    string = strdup("<list>");
                    str_len = 6;
                    break;
            }
            break;
        }
        case BOOLEAN: {
            if (value.p_value) {
                string = strdup("true");
                str_len = 4;
            } else {
                string = strdup("false");
                str_len = 5;
            }
            break;
        }
        case NIL:
            string = strdup("nil");
            str_len = 3;
            break;
    }

    *dest = string;
    return str_len;
}

static void print_object(Object *object, const char *new_line, bool print_quotation) {
    switch (object->type) {
        case OBJ_STRING: {
            const char *quotation = print_quotation ? "\"" : "";
            ObjString *string = (ObjString *) object;
            printf("%s%.*s%s%s", quotation, (int) string->length, string->start, quotation, new_line);
            break;
        }
        case OBJ_LAMBDA: {
            ObjLambda *lambda = (ObjLambda *) object;
            printf("<function of arity %d>%s", lambda->num_params, new_line);
            break;
        }
        case OBJ_NATIVE_FUNC: {
            ObjNativeFunc *n_fn = (ObjNativeFunc *) object;
            printf("<native function of arity %d>%s", n_fn->num_params, new_line);
            break;
        }
        case OBJ_DICT: {
            print_dict((ObjDict *) object, false);
            printf("%s", new_line);
            break;
        }
        default:
            printf("Invalid object%s", new_line);
    }
}

void print_value(CrispyValue value, bool new_line, bool print_quotation) {
    const char *nl = (new_line) ? "\n" : "";
    switch (value.type) {
        case NUMBER:
            printf("%.15g%s", value.d_value, nl);
            break;
        case BOOLEAN:
            printf("%s%s", BOOL_STRING(value), nl);
            break;
        case OBJECT:
            print_object(value.o_value, nl, print_quotation);
            break;
        case NIL:
            printf("nil%s", nl);
            break;
        default:
            printf("Invalid value%s", nl);
            break;
    }
}

CrispyValue create_nil() {
    CrispyValue val;
    val.type = NIL;
    val.p_value = 0;
    return val;
}

CrispyValue create_number(double value) {
    CrispyValue val;
    val.type = NUMBER;
    val.d_value = value;
    return val;
}

CrispyValue create_bool(bool value) {
    CrispyValue val;
    val.type = BOOLEAN;
    val.p_value = (value) ? 1 : 0;
    return val;
}

CrispyValue create_object(Object *object) {
    CrispyValue val;
    val.type = OBJECT;
    val.o_value = object;
    return val;
}

void print_object_type(CrispyValue value) {
    Object *object = value.o_value;

    switch (object->type) {
        case OBJ_STRING:
            printf(" : STRING\n");
            break;
        case OBJ_LAMBDA:
            printf(" : LAMBDA\n");
            break;
        case OBJ_DICT:
            printf(" : DICT\n");
            break;
        default:
            printf(" : OBJECT\n");
            break;
    }
}

void print_type(CrispyValue value) {
    switch (value.type) {
        case NUMBER:
            printf(" : NUMBER\n");
            break;
        case BOOLEAN:
            printf(" : BOOLEAN\n");
            break;
        case OBJECT:
            print_object_type(value);
            break;
        case NIL:
            printf(" : NIL\n");
            break;
    }
}

void code_buff_init(CodeBuffer *code_buffer) {
    code_buffer->cap = 0;
    code_buffer->count = 0;
    code_buffer->code = NULL;
}

void code_buff_free(CodeBuffer *code_buffer) {
    FREE_ARR(code_buffer->code);
    code_buff_init(code_buffer);
}

CallFrame *new_temp_call_frame(CallFrame *other) {
    CallFrame *call_frame = malloc(sizeof(CallFrame));
    call_frame->code_buffer = other->code_buffer;
    call_frame->ip = other->ip;

    ValueArray variables;
    val_arr_init(&variables);
    call_frame->variables = variables;

    call_frame->constants = other->constants;

    return call_frame;
}

void temp_call_frame_free(CallFrame *call_frame) {
    val_arr_free(&call_frame->variables);
    free(call_frame);
}

CallFrame *new_call_frame() {
    CallFrame *call_frame = malloc(sizeof(CallFrame));
    call_frame->ip = NULL;

    CodeBuffer code_buffer;
    code_buff_init(&code_buffer);
    call_frame->code_buffer = code_buffer;

    ValueArray variables;
    val_arr_init(&variables);
    call_frame->variables = variables;

    ValueArray constants;
    val_arr_init(&constants);
    call_frame->constants = constants;

    return call_frame;
}

void call_frame_free(CallFrame *call_frame) {
    code_buff_free(&call_frame->code_buffer);

    val_arr_free(&call_frame->variables);
    val_arr_free(&call_frame->constants);

    free(call_frame);
}

#define ALLOC_OBJ(vm, type, object_type) ((type *)allocate_object((vm), sizeof(type), (object_type)))

/**
 * Allocates a new Object on the heap.
 * Also adds it to the linked list of objects inside the VM and performs garbage collection,
 * if the threshold of allocated objects is surpassed.
 * @param vm the current VM.
 * @param size the size, that needs to be allocated in bytes.
 * @param type the type of the object.
 * @return a pointer to the created object.
 */
static Object *allocate_object(Vm *vm, size_t size, ObjectType type) {
    // TODO don't gc while compiling
    if (vm->allocated_mem >= vm->max_alloc_mem) {
        gc(vm);
    }

    Object *object = malloc(size);
    object->type = type;
    object->marked = false;

    object->next = vm->first_object;
    vm->first_object = object;
    vm->allocated_mem += size;

#if DEBUG_TRACE_GC
    printf("[%p] Allocated %ld bytes for object of type %d\n", object, size, type);
#endif

    return object;
}

ObjString *new_string(Vm *vm, const char *start, size_t length) {
    ObjString *string = ALLOC_OBJ(vm, ObjString, OBJ_STRING);
    string->length = length;
    string->hashed = false;

    size_t size = length * sizeof(char);
    char *value = malloc(size);
    memcpy(value, start, length);

    vm->allocated_mem += size;

#if DEBUG_TRACE_GC
    printf("[%p] Allocated %ld bytes for string\n", string, size);
#endif

    string->start = value;
    return string;
}

ObjString *new_empty_string(Vm *vm, size_t length) {
    ObjString *string = ALLOC_OBJ(vm, ObjString, OBJ_STRING);
    string->length = length;
    string->hashed = false;

    size_t size = length * sizeof(char);
    char *value = malloc(size);
    string->start = value;

    vm->allocated_mem += size;

#if DEBUG_TRACE_GC
    printf("[%p] Allocated %ld bytes for string\n", string, size);
#endif

    return string;
}

ObjLambda *new_lambda(Vm *vm, uint8_t num_params) {
    ObjLambda *lambda = ALLOC_OBJ(vm, ObjLambda, OBJ_LAMBDA);
    lambda->num_params = num_params;

    lambda->call_frame = NULL;

    return lambda;
}

ObjNativeFunc *new_native_func(Vm *vm, void *func_ptr, uint8_t num_args, bool system_func) {
    ObjNativeFunc *n_fn = ALLOC_OBJ(vm, ObjNativeFunc, OBJ_NATIVE_FUNC);
    n_fn->func_ptr = func_ptr;
    n_fn->num_params = num_args;
    n_fn->system_func = system_func;

    return n_fn;
}

ObjDict *new_dict(Vm *vm, HashTable content) {
    ObjDict *dict = ALLOC_OBJ(vm, ObjDict, OBJ_DICT);
    dict->content = content;

    return dict;
}

uint32_t hash_string(const char *string, size_t length) {
    //
    uint32_t hash = 5381;

    for (uint32_t i = 0; i < length; ++i) {
        char c = string[i];
        hash = hash * 33 ^ c;
    }

    return hash;
}

uint32_t hash_string_obj(ObjString *string) {
    if (string->hashed) {
        return string->hash;
    }

    uint32_t hash = hash_string(string->start, string->length);

    string->hash = hash;
    string->hashed = true;

    return hash;
}

uint32_t hash_uint32_t(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

int cmp_strings(ObjString *first, ObjString *second) {
    if (first == second) {
        return 0;
    }

    size_t smaller_length = (first->length < second->length) ? first->length : second->length;
    return memcmp(first->start, second->start, smaller_length);
}

int cmp_values(CrispyValue first, CrispyValue second) {
    if (first.type != second.type) {
        return 1;
    }

    switch (first.type) {
        case NUMBER:
            if (first.d_value == second.d_value) {
                return 0;
            }
            return first.d_value < second.d_value ? -1 : 1;
        case OBJECT:
            return cmp_objects(first.o_value, second.o_value);
        case BOOLEAN:
            if (first.p_value == second.p_value) {
                return 0;
            }
            return first.p_value < second.p_value ? -1 : 1;
        case NIL:
            return second.type != NIL;
    }

    return 1;
}

int cmp_objects(Object *first, Object *second) {
    if (first->type != second->type) {
        return 1;
    }

    switch (first->type) {
        case OBJ_STRING:
            return cmp_strings((ObjString *) first, (ObjString *) second);
        case OBJ_DICT:
            // TODO implement
            break;
        case OBJ_LIST:
            // TODO implement
            break;
        default:
            return 1;
    }

    return 1;
}
