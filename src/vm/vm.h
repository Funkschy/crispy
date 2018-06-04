#ifndef VM_H
#define VM_H

#define CURR_FRAME(vm_ptr) ((vm_ptr)->frames[(vm_ptr)->frame_count - 1])

#include "../cli/common.h"
#include "value.h"
#include "../compiler/compiler.h"
#include "options.h"

typedef enum {
    INTERPRET_OK,
    INTERPRET_RUNTIME_ERROR,
    INTERPRET_COMPILE_ERROR
} InterpretResult;

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

typedef struct {
    Value stack[STACK_MAX];
    Value *sp;

    CallFrame frames[FRAMES_MAX];
    size_t frame_count;

    Compiler compiler;

    size_t num_objects;
    size_t max_objects;
    Object *first_object;
} Vm;

void gc(Vm *vm);

void init_vm(Vm *vm);

void free_vm(Vm *vm);

void compile(Vm *vm);

void free_object(Object *object);

void pop_call_frame(Vm *vm);

void push_call_frame(Vm *vm);

void write_code_buffer(CodeBuffer *code_buffer, uint8_t instruction);

uint32_t add_constant(CodeBuffer *code_buffer, Value value);

ObjString *new_empty_string(Vm *vm, size_t length);

ObjString *new_string(Vm *vm, const char *start, size_t length);

InterpretResult interpret(Vm *vm, const char *source);

#endif