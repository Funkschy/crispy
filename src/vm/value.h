#ifndef VALUE_H
#define VALUE_H

#include "../cli/common.h"

#define CHECK_TYPE(val, check_type) ((val).type == (check_type))

#define CHECK_NUM(num) (CHECK_TYPE((num), NUMBER))
#define CHECK_BOOL(bool_val) (CHECK_TYPE((bool_val), BOOLEAN))

#define BOOL_TRUE(bool_val) ((bool_val).p_value == 1)
#define BOOL_STRING(bool_val) (((bool_val).p_value == 1) ? "true" : "false")

typedef struct object_t Object;

typedef enum {
    NUMBER, OBJECT, BOOLEAN, NIL
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

} CodeBuffer;

typedef struct s_call_frame {
    uint8_t *ip;

    CodeBuffer code_buffer;

    ValueArray variables;
    ValueArray constants;
} CallFrame;

typedef enum {
    OBJ_STRING,
    OBJ_LAMBDA,
    OBJ_NATIVE_FUNC,
    OBJ_MAP,
    OBJ_LIST
} ObjectType;

struct object_t {
    uint8_t marked;
    ObjectType type;

    struct object_t *next;
};

typedef struct {
    Object object;

    size_t length;
    const char *start;

    bool hashed;
    uint32_t hash;
} ObjString;

typedef struct {
    Object object;

    size_t num_params;
    CallFrame call_frame;
} ObjLambda;

typedef struct {
    Object object;

    size_t num_params;
    void *func_ptr;
} ObjNativeFunc;

Value create_nil();

Value create_bool(bool value);

Value create_number(double value);

Value create_object(Object *object);

// TODO rename to type_init etc
void init_call_frame(CallFrame *call_frame);

void free_call_frame(CallFrame *call_frame);

void init_value_array(ValueArray *value_array);

void free_value_array(ValueArray *value_array);

void write_value(ValueArray *value_array, Value value);

void write_at(ValueArray *value_array, uint8_t index, Value value);

void print_value(Value value, bool new_line);

void print_type(Value value);

int comp_string(ObjString *first, ObjString *second);

/**
 * Computes the hash value for an unsigned 4 byte integer.
 * See https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key/12996028#12996028.
 * @param x the integer that should be hashed.
 * @return the hash.
 */
uint32_t hash_uint32_t(uint32_t x);

/**
 * Computes the hash value for the given string und cashes it in the string object.
 * @param string the string object.
 * @return the hash.
 */
uint32_t hash_string_obj(ObjString *string);

/**
 * Uses the djb2 hash algorithm by Dan Bernstein to hash a string.
 * @param string the string to be hashed (does not need to be null terminated).
 * @param length the length of the supplied string.
 * @return the computed hash.
 */
uint32_t hash_string(const char *string, size_t length);

#endif