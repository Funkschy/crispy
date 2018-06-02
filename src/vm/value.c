#include <stdio.h>

#include "value.h"
#include "object.h"
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
    if(value_array->count >= value_array->cap) {
        value_array->cap = GROW_CAP(value_array->cap);
        value_array->values = GROW_ARR(value_array->values, Value, value_array->cap);
    }
    
    value_array->values[value_array->count++] = value;
}

void write_at(ValueArray *value_array, uint8_t index, Value value) {
    while(true) {
        if(index < value_array->cap) {
            value_array->values[index] = value;
            if(index > value_array->count) value_array->count = index;
            return;
        }
        
        
        value_array->cap = GROW_CAP(value_array->cap);
        value_array->values = GROW_ARR(value_array->values, Value, value_array->cap);
    }
}

static void print_object(Object *object, const char *new_line) {
    switch (object->type) {
        case OBJ_STRING: {
            ObjString *string = (ObjString *)object;
            printf("%.*s%s", (int) string->length, string->start, new_line);
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

void print_type(Value value) {
    switch (value.type) {
        case NUMBER:
            printf(" : NUMBER\n");
            break;
        case BOOLEAN:
            printf(" : BOOLEAN\n");
            break;
        case OBJECT:
            printf(" : OBJECT\n");
            break;
    }
}
