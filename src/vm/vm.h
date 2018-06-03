#ifndef VM_H
#define VM_H

#define DEBUG_TRACE_GC 0

#define DEBUG_TRACE_EXECUTION 0
#define DEBUG_SHOW_DISASSEMBLY 0

#define INITIAL_GC_THRESHOLD 32

#include "../cli/common.h"
#include "value.h"
#include "object.h"

#define STACK_MAX 256

typedef enum {
    INTERPRET_OK,
    INTERPRET_RUNTIME_ERROR,
    INTERPRET_COMPILE_ERROR
} InterpretResult;

struct s_vm {
    uint8_t *ip;

    Value stack[STACK_MAX];
    Value *sp;

    uint32_t instruction_cap;
    uint32_t instruction_count;
    uint8_t *code;

    ValueArray variables;
    ValueArray constants;

    size_t num_objects;
    size_t max_objects;
    Object *first_object;
};

void init_vm(Vm *vm);

void free_vm(Vm *vm);

void write_code(Vm *vm, uint8_t instruction);

uint32_t add_constant(Vm *vm, Value value);

InterpretResult interpret(Vm *vm, const char *source);

#endif