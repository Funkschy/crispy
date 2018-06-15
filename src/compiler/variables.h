// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef CALC_VARIABLES_H
#define CALC_VARIABLES_H

#include "../cli/common.h"

typedef struct {
    const char *name;
    size_t length;
    int index;
    int frame_offset;
    bool assignable;
} Variable;

typedef struct {
    uint32_t cap;
    uint32_t count;
    Variable *variables;
} VariableArray;

void init_variable_array(VariableArray *variable_array);
void free_variable_array(VariableArray *variable_array);
uint32_t  write_variable(VariableArray *variable_array, Variable variable);

#endif //CALC_VARIABLES_H
