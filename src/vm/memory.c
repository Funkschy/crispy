// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>

#include "memory.h"
#include "value.h"
#include "vm.h"

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
    for (int i = vm->frame_count - 1; i >= 0; --i) {
        CallFrame *curr_frame = vm->frames.frame_pointers[i];

        // variables
        for (int j = 0; j < curr_frame->variables.count; ++j) {
            CrispyValue *value = &curr_frame->variables.values[j];

            if (value->type == OBJECT) {
                mark(value->o_value);
            }
        }

        // constants
        for (int j = 0; j < curr_frame->constants.count; ++j) {
            CrispyValue *value = &curr_frame->constants.values[j];

            if (value->type == OBJECT) {
                mark(value->o_value);
            }
        }
    }
}

static void sweep(Vm *vm) {
    Object **object = &vm->first_object;
    while (*object) {
        if (!(*object)->marked) {
            Object *unreached = *object;
            *object = unreached->next;

            vm->allocated_mem -= free_object(unreached);
        } else {
            (*object)->marked = 0;
            object = &(*object)->next;
        }
    }
}

void gc(Vm *vm) {
#if DEBUG_TRACE_GC
    size_t mem_before = vm->allocated_mem;
#endif

    mark_all(vm);
    sweep(vm);

    vm->max_alloc_mem = vm->allocated_mem * 2;

#if DEBUG_TRACE_GC
    printf("Collected %ld bytes, %ld remaining.\n", mem_before - vm->allocated_mem, vm->allocated_mem);
#endif
}


