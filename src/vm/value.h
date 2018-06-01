#ifndef VALUE_H
#define VALUE_H

#include "../cli/common.h"
#include "memory.h"

typedef uint64_t Value;

typedef struct {
    uint32_t cap;
    uint32_t count;
    Value *values;
} ValueArray;

void init_value_array(ValueArray *value_array);
void free_value_array(ValueArray *value_array);
void write_value(ValueArray *value_array, Value value);
void write_at(ValueArray *value_array, uint8_t index, Value value);
void print_value(Value value);

#endif