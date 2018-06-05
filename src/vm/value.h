#ifndef VALUE_H
#define VALUE_H

#include "../cli/common.h"

#define CHECK_TYPE(val, check_type) ((val).type == (check_type))

#define CHECK_NUM(num) (CHECK_TYPE((num), NUMBER))
#define CHECK_BOOL(bool_val) (CHECK_TYPE((bool_val), BOOLEAN))

#define BOOL_TRUE(bool_val) ((bool_val).p_value == 1)
#define BOOL_STRING(bool_val) (((bool_val).p_value == 1) ? "true" : "false")

typedef struct s_object Object;

typedef enum {
    NUMBER, OBJECT, BOOLEAN
} ValueType;

typedef struct {
    ValueType type;
    union {
        double d_value;     // number
        uint64_t p_value;   // primitive value (e.g. boolean)
        Object *o_value;    // object pointer
    };
} Value;

typedef struct {
    uint32_t cap;
    uint32_t count;
    Value *values;
} ValueArray;

typedef struct {
    uint32_t cap;
    uint32_t count;
    uint8_t *code;

    ValueArray variables;
    ValueArray constants;
} CodeBuffer;

typedef struct s_call_frame{
    uint8_t *ip;

    CodeBuffer code_buffer;
} CallFrame;

typedef enum {
    OBJ_STRING,
    OBJ_LAMBDA,
    OBJ_MAP,
    OBJ_LIST
} ObjectType;

struct s_object {
    uint8_t marked;
    ObjectType type;

    struct s_object *next;
};

typedef struct {
    Object object;

    size_t length;
    const char *start;
} ObjString;

typedef struct {
    Object object;

    size_t num_params;
    CallFrame call_frame;
} ObjLambda;

Value create_bool(bool value);

Value create_number(double value);

Value create_object(Object *object);

void init_call_frame(CallFrame *call_frame);

void init_value_array(ValueArray *value_array);

void free_value_array(ValueArray *value_array);

void write_value(ValueArray *value_array, Value value);

void write_at(ValueArray *value_array, uint8_t index, Value value);

void print_value(Value value, bool new_line);

void print_type(Value value);

#endif