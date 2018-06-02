#include <stdio.h>

#include "memory.h"
#include "value.h"
#include "object.h"

void *reallocate(void *previous, size_t size) {
    if (size <= 0) {
        free(previous);
        return NULL;
    }

    return realloc(previous, size);
}

static void mark(Object *object) {
    if (object->marked) return;

    object->marked = 1;
}

static void mark_all(Vm *vm) {
    // TODO
}

static void sweep(Vm *vm) {
    Object **object = &vm->first_object;
    while (*object) {
        if (!(*object)->marked) {
            Object *unreached = *object;
            *object = unreached->next;
            --vm->num_objects;

            free(unreached);
        } else {
            (*object)->marked = 0;
            object = &(*object)->next;
        }
    }
}

void gc(Vm *vm) {
    mark_all(vm);
    sweep(vm);
}


