#ifndef CALC_OBJECT_H
#define CALC_OBJECT_H

#include "../cli/common.h"
#include "vm.h"

typedef enum {
    OBJ_STRING,
    OBJ_LAMBDA,
    OBJ_MAP,
    OBJ_LIST
} ObjectType;

struct s_object {
    uint8_t marked;
    ObjectType type;

    struct s_object *next;
};

struct s_obj_string {
    Object object;
    size_t length;
    const char *start;
};

ObjString *new_string(Vm *vm, const char *start, size_t length);

#endif //CALC_OBJECT_H