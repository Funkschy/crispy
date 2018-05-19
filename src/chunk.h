#ifndef CHUNK_H
#define CHUNK_H

#include "common.h"
#include "value.h"
#include "memory.h"

typedef enum {
    OP_ADD,         // add two numbers
    OP_SUB,         // subtract two numbers
    OP_MUL,         // multiply two numbers
    OP_DIV,         // divide two numbers
    OP_LDC,         // load number from constant pool
    OP_LDC_0,       // Load 0 as a constant
    OP_LDC_1,       // Load 1 as a constant
    OP_NEGATE,      // negate a number
    OP_STORE,       // store number as variable
    OP_LOAD,        // load variable to stack
    OP_DUP,         // duplicate
    OP_PRINT,       // print
    OP_POP,         // pop from stack
    OP_JMP,         // unconditional jump
    OP_JEQ,         // jump if equals
    OP_JNE,         // jump if not equals
    OP_JLT,         // jump if less than
    OP_JLE,         // jump if equals or less
    OP_JGT,         // jump if equals or greater
    OP_JGE,         // jump if greater
    OP_INC,         // increment
    OP_DEC,         // decrement
    OP_RETURN       // return from Chunk
} OP_CODE;

typedef struct {
    uint32_t cap;
    uint32_t count;
    uint8_t *code;

    ValueArray variables;
    ValueArray constants;
} Chunk;

void init_chunk(Chunk *chunk);

void init_chunk_direct(Chunk *chunk, uint8_t *code, size_t size);

void free_chunk(Chunk *chunk);

void write_chunk(Chunk *chunk, uint8_t instruction);

uint32_t add_constant(Chunk *chunk, Value value);

#endif