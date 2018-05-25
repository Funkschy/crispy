#ifndef CHUNK_H
#define CHUNK_H

#include "common.h"
#include "value.h"
#include "memory.h"

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