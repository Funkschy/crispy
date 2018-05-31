#ifndef VM_H
#define VM_H

#include "../cli/common.h"
#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef enum {
    INTERPRET_OK,
    INTERPRET_RUNTIME_ERROR,
    INTERPRET_COMPILE_ERROR
} InterpretResult;

typedef struct {
    uint8_t *ip;
} StackFrame;

typedef struct {
    Chunk *chunk;
    uint8_t *ip;
    
    Value stack[STACK_MAX];
    Value *sp;
} Vm;

void init_vm(Vm *vm);
void free_vm(Vm *vm);

InterpretResult interpret(Vm *vm, const char *source);
void push(Vm *vm, Value value);
Value pop(Vm *vm);
Value peek(Vm *vm);

#endif