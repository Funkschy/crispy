#include <stdio.h>

#include "memory.h"
#include "value.h"

void *reallocate(void *previous, size_t size) {
    if (size <= 0) {
        free(previous);
        return NULL;
    }

    return realloc(previous, size);
}

void gc(Vm *vm) {

}


