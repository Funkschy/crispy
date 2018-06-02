#ifndef CALC_COMPILER_H
#define CALC_COMPILER_H

#include "../vm/vm.h"
#include "../vm/chunk.h"

void compile(const char *source, Vm *vm, Chunk *chunk);

#endif //CALC_COMPILER_H
