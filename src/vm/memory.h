#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include "chunk.h"
#include "object.h"

#define GROW_CAP(old_cap) (old_cap) < 8 ? 8 : ((old_cap) * 2)
#define FREE_ARR(previous) reallocate(previous, 0)
#define GROW_ARR(previous, type, new_cap) \
        reallocate(previous, sizeof(type) * (new_cap))

void *reallocate(void *previous, size_t size);

void gc(Vm *vm);

#endif