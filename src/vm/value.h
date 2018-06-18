// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

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
} CrispyValue;

typedef struct {
    uint32_t cap;
    uint32_t count;
    CrispyValue *values;
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
    OBJ_DICT,
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

    uint8_t num_params;
    CallFrame *call_frame;
} ObjLambda;

typedef struct {
    Object object;

    // determines if the current vm should be passed to the function
    // only used for standard lib
    bool system_func;
    uint8_t num_params;
    void *func_ptr;
} ObjNativeFunc;

CrispyValue create_nil();

CrispyValue create_bool(bool value);

CrispyValue create_number(double value);

CrispyValue create_object(Object *object);

CallFrame *new_call_frame();

CallFrame *new_temp_call_frame(CallFrame *other);

void temp_call_frame_free(CallFrame *call_frame);

void call_frame_free(CallFrame *call_frame);

void val_arr_init(ValueArray *value_array);

void code_buff_init(CodeBuffer *code_buffer);

void val_arr_free(ValueArray *value_array);

void code_buff_free(CodeBuffer *code_buffer);

void write_value(ValueArray *value_array, CrispyValue value);

void write_at(ValueArray *value_array, uint32_t index, CrispyValue value);

void print_value(CrispyValue value, bool new_line, bool print_quotation);

void print_type(CrispyValue value);

int cmp_values(CrispyValue first, CrispyValue second);

int cmp_objects(Object *first, Object *second);

int cmp_strings(ObjString *first, ObjString *second);

/**
 * Creates a heap allocated string from any value.
 * @param value the value that will be stringified.
 * @param dest the pointer that will point to the created string.
 * @return the length of the created string.
 */
size_t value_to_string(CrispyValue value, char **dest);

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