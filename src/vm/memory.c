// Copyright (c) 2018 Felix Schoeller
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
            Value *value = &curr_frame->variables.values[j];

            if (value->type == OBJECT) {
                mark(value->o_value);
            }
        }

        // constants
        for (int j = 0; j < curr_frame->constants.count; ++j) {
            Value *value = &curr_frame->constants.values[j];

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


