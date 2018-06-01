#include <stdio.h>

#include "value.h"

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

void print_value(Value value) {
    printf("%li", value);
}