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
    CodeBuffer *curr_buffer = &CURR_FRAME(vm).code_buffer;

    // variables
    for (int i = 0; i < curr_buffer->variables.count; ++i) {
        Value *value = &curr_buffer->variables.values[i];

        if (value->type == OBJECT) {
            mark(value->o_value);
        }
    }

    // constants
    for (int i = 0; i < curr_buffer->constants.count; ++i) {
        Value *value = &curr_buffer->constants.values[i];

        if (value->type == OBJECT) {
            mark(value->o_value);
        }
    }

    /*
    // stack
    size_t stack_size = vm->sp - vm->stack;
    for(int i = 0; i < stack_size; ++i) {
        Value *value = &vm->stack[i];

        if(value->type == OBJECT) {
            mark(value->o_value);
        }
    }
     */
}

static void sweep(Vm *vm) {
    Object **object = &vm->first_object;
    while (*object) {
        if (!(*object)->marked) {
            Object *unreached = *object;
            *object = unreached->next;
            --vm->num_objects;

            free_object(unreached);
        } else {
            (*object)->marked = 0;
            object = &(*object)->next;
        }
    }
}

void gc(Vm *vm) {
    size_t objects_before = vm->num_objects;

    mark_all(vm);
    sweep(vm);

#if DEBUG_TRACE_GC
    printf("Collected %ld objects, %ld remaining.\n", objects_before - vm->num_objects, vm->num_objects);
#endif
}


