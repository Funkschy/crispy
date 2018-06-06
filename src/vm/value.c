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

void write_at(ValueArray *value_array, uint8_t index, Value value) {
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
    }
}

static void init_code_buffer(CodeBuffer *code_buffer) {
    code_buffer->cap = 0;
    code_buffer->count = 0;
    code_buffer->code = NULL;

    init_value_array(&code_buffer->constants);
    init_value_array(&code_buffer->variables);
}

static void free_code_buffer(CodeBuffer *code_buffer) {
    free(code_buffer->code);

    free_value_array(&code_buffer->constants);
    free_value_array(&code_buffer->variables);
}

void init_call_frame(CallFrame *call_frame) {
    call_frame->ip = NULL;
    init_code_buffer(&call_frame->code_buffer);
}

void free_call_frame(CallFrame *call_frame) {
    free_code_buffer(&call_frame->code_buffer);
}

#define ALLOC_OBJ(vm, type, object_type) ((type *)allocate_object((vm), sizeof(type), (object_type)))

static Object *allocate_object(Vm *vm, size_t size, ObjectType type) {
    // TODO don't gc while compiling
    if (vm->num_objects >= vm->max_objects) gc(vm);

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

    char *value = malloc(length * sizeof(char));
    memcpy(value, start, length);

    string->start = value;
    return string;
}

ObjString *new_empty_string(Vm *vm, size_t length) {
    ObjString *string = ALLOC_OBJ(vm, ObjString, OBJ_STRING);
    string->length = length;

    char *value = malloc(length * sizeof(char));
    string->start = value;
    return string;
}

ObjLambda *new_lambda(Vm *vm, size_t num_params) {
    ObjLambda *lambda = ALLOC_OBJ(vm, ObjLambda, OBJ_LAMBDA);
    lambda->num_params = num_params;

    CallFrame call_frame;
    init_call_frame(&call_frame);
    lambda->call_frame = call_frame;

    return lambda;
}
