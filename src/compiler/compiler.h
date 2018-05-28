#ifndef CALC_COMPILER_H
#define CALC_COMPILER_H

#include "../vm/chunk.h"

void compile(const char *source, Chunk *chunk);

#endif //CALC_COMPILER_H
