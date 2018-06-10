#include <stdio.h>
#include <string.h>

#include "value.h"
#include "vm.h"
#include "memory.h"

void init_value_array(ValueArray *value_array) {
    value_array->cap = 0;
    value_array->count = 0;
    value_array->values = NULL;
}

void free_value_array(ValueArray *value_array) {
    FREE_ARR(value_array->values);
    init_value_array(value_array);
}

void write_value(ValueArray *value_array, Value value) {
    if (value_array->count >= value_array->cap) {
        value_array->cap = GROW_CAP(value_array->cap);
        value_array->values = GROW_ARR(value_array->values, Value, value_array->cap);
    }

    value_array->values[value_array->count++] = value;
}

void write_at(ValueArray *value_array, uint32_t index, Value value) {
    while (index >= value_array->cap) {
        value_array->cap = GROW_CAP(value_array->cap);
        value_array->values = GROW_ARR(value_array->values, Value, value_array->cap);
    }

    if (index >= value_array->count) {
        ++value_array->count;
    }

    value_array->values[index] = value;
}

static void print_object(Object *object, const char *new_line) {
    switch (object->type) {
        case OBJ_STRING: {
            ObjString *string = (ObjString *) object;
            printf("%.*s%s", (int) string->length, string->start, new_line);
            break;
        }
        case OBJ_LAMBDA: {
            ObjLambda *lambda = (ObjLambda *) object;
            printf("<function of arity %ld>%s", lambda->num_params, new_line);
            break;
        }
        case OBJ_NATIVE_FUNC: {
            printf("<native function of arity 1>%s", new_line);
            break;
        }
        default:
            printf("Invalid object%s", new_line);
    }
}

void print_value(Value value, bool new_line) {
    const char *nl = (new_line) ? "\n" : "";
    switch (value.type) {
        case NUMBER:
            printf("%f%s", value.d_value, nl);
            break;
        case BOOLEAN:
            printf("%s%s", BOOL_STRING(value), nl);
            break;
        case OBJECT:
            print_object(value.o_value, nl);
            break;
        case NIL:
            printf("nil%s", nl);
            break;
        default:
            printf("Invalid value%s", nl);
            break;
    }
}

Value create_nil() {
    Value val;
    val.type = NIL;
    val.p_value = 0;
    return val;
}

Value create_number(double value) {
    Value val;
    val.type = NUMBER;
    val.d_value = value;
    return val;
}

Value create_bool(bool value) {
    Value val;
    val.type = BOOLEAN;
    val.p_value = (value) ? 1 : 0;
    return val;
}

Value create_object(Object *object) {
    Value val;
    val.type = OBJECT;
    val.o_value = object;
    return val;
}

void print_object_type(Value value) {
    Object *object = value.o_value;

    switch (object->type) {
        case OBJ_STRING:
            printf(" : STRING\n");
            break;
        case OBJ_LAMBDA:
            printf(" : LAMBDA\n");
            break;
        default:
            printf(" : OBJECT\n");
            break;
    }
}

void print_type(Value value) {
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

static void init_code_buffer(CodeBuffer *code_buffer) {
    code_buffer->cap = 0;
    code_buffer->count = 0;
    code_buffer->code = NULL;
}

static void free_code_buffer(CodeBuffer *code_buffer) {
    FREE_ARR(code_buffer->code);
    init_code_buffer(code_buffer);
}

CallFrame *new_temp_call_frame(CallFrame *other) {
    CallFrame *call_frame = malloc(sizeof(CallFrame));
    call_frame->code_buffer = other->code_buffer;
    call_frame->ip = other->ip;

    ValueArray variables;
    init_value_array(&variables);
    call_frame->variables = variables;

    call_frame->constants = other->constants;

    return call_frame;
}

void free_temp_call_frame(CallFrame *call_frame) {
    free_value_array(&call_frame->variables);
    free(call_frame);
}

CallFrame *new_call_frame() {
    CallFrame *call_frame = malloc(sizeof(CallFrame));
    call_frame->ip = NULL;

    CodeBuffer code_buffer;
    init_code_buffer(&code_buffer);
    call_frame->code_buffer = code_buffer;

    ValueArray variables;
    init_value_array(&variables);
    call_frame->variables = variables;

    ValueArray constants;
    init_value_array(&constants);
    call_frame->constants = constants;

    return call_frame;
}

void free_call_frame(CallFrame *call_frame) {
    free_code_buffer(&call_frame->code_buffer);

    free_value_array(&call_frame->variables);
    free_value_array(&call_frame->constants);

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
    if (vm->num_objects >= vm->max_objects) { gc(vm); }

    Object *object = malloc(size);
    object->type = type;
    object->marked = false;

    object->next = vm->first_object;
    vm->first_object = object;
    ++vm->num_objects;

#if DEBUG_TRACE_GC
    printf("[%p] Allocated %ld bytes for object of type %d\n", object, size, type);
#endif

    return object;
}

ObjString *new_string(Vm *vm, const char *start, size_t length) {
    ObjString *string = ALLOC_OBJ(vm, ObjString, OBJ_STRING);
    string->length = length;
    string->hashed = false;

    char *value = malloc(length * sizeof(char));
    memcpy(value, start, length);

    string->start = value;
    return string;
}

ObjString *new_empty_string(Vm *vm, size_t length) {
    ObjString *string = ALLOC_OBJ(vm, ObjString, OBJ_STRING);
    string->length = length;
    string->hashed = false;

    char *value = malloc(length * sizeof(char));
    string->start = value;
    return string;
}

ObjLambda *new_lambda(Vm *vm, size_t num_params) {
    ObjLambda *lambda = ALLOC_OBJ(vm, ObjLambda, OBJ_LAMBDA);
    lambda->num_params = num_params;

    lambda->call_frame = NULL;

    return lambda;
}

ObjNativeFunc *new_native_func(Vm *vm, void *func_ptr) {
    ObjNativeFunc *n_fn = ALLOC_OBJ(vm, ObjNativeFunc, OBJ_NATIVE_FUNC);
    n_fn->func_ptr = func_ptr;

    return n_fn;
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

int comp_string(ObjString *first, ObjString *second) {
    size_t smaller_length = (first->length < second->length) ? first->length : second->length;
    return memcmp(first->start, second->start, smaller_length);
}
