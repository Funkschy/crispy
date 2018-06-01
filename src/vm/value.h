#ifndef VALUE_H
#define VALUE_H

#include "../cli/common.h"
#include "memory.h"

#define CHECK_TYPE(val, check_type) ((val).type == (check_type))

#define CHECK_NUM(num) (CHECK_TYPE((num), NUMBER))
#define CHECK_BOOL(bool_val) (CHECK_TYPE((bool_val), BOOLEAN))

#define BOOL_TRUE(bool_val) ((bool_val).p_value == 1)
#define BOOL_STRING(bool_val) (((bool_val).p_value == 1) ? "true" : "false")

typedef enum {
    NUMBER, OBJECT, BOOLEAN
} ValueType;

typedef struct {
    ValueType type;
    union {
        double d_value;     // number
        uint64_t p_value;   // primitive value (e.g. boolean)
        char *o_value;      // object pointer
    };
} Value;

typedef struct {
    uint32_t cap;
    uint32_t count;
    Value *values;
} ValueArray;

Value create_number(double value);
Value create_bool(bool value);

void init_value_array(ValueArray *value_array);
void free_value_array(ValueArray *value_array);
void write_value(ValueArray *value_array, Value value);
void write_at(ValueArray *value_array, uint8_t index, Value value);
void print_value(Value value, bool new_line);
void print_type(Value value);

#endif