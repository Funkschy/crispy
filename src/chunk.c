#include <tgmath.h>

#include "chunk.h"

void init_chunk(Chunk *chunk) {
    chunk->cap = 0;
    chunk->count = 0;
    chunk->code = NULL;
    
    init_value_array(&chunk->constants);
    init_value_array(&chunk->variables);
}

void free_chunk(Chunk *chunk) {
    FREE_ARR(chunk->code);
    free_value_array(&chunk->constants);
    free_value_array(&chunk->variables);
    init_chunk(chunk);
}

void write_chunk(Chunk *chunk, uint8_t instruction) {
    if(chunk->count >= chunk->cap) {
        chunk->cap = GROW_CAP(chunk->cap);
        chunk->code = GROW_ARR(chunk->code, uint8_t, chunk->cap);
    }
    
    chunk->code[chunk->count++] = instruction;
}

uint32_t add_constant(Chunk *chunk, Value value) {
    write_value(&chunk->constants, value);
    return chunk->constants.count - 1;
}

void init_chunk_direct(Chunk *chunk, uint8_t *code, size_t size) {
    // find next biggest multiple of 2
    uint32_t cap = (uint32_t)pow(2, ceil(log2(size)));

    chunk->code = code;
    chunk->cap = cap;
    chunk->count = (uint32_t) size;
}
