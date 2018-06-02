#include <stdlib.h>
#include <stdio.h>

#include "object.h"
#include "memory.h"

#define ALLOC_OBJ(vm, type, object_type) ((type *)allocate_object((vm), sizeof(type), (object_type)))

static Object *allocate_object(Vm *vm, size_t size, ObjectType type) {
    if(vm->num_objects >= vm->max_objects) gc(vm);

    Object *object = malloc(size);
    object->type = type;
    object->marked = false;

    object->next = vm->first_object;
    vm->first_object = object;
    ++vm->num_objects;

#if DEBUG_TRACE_GC
    printf("[%p] Allocated %ld bytes for object of type %d\n", object, size, type);
#endif

    return object;
}

ObjString *new_string(Vm *vm, const char *start, size_t length) {
    ObjString *string = ALLOC_OBJ(vm, ObjString, OBJ_STRING);
    string->length = length;
    string->start = start;

    return string;
}
