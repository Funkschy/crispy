#ifndef CALC_VARIABLES_H
#define CALC_VARIABLES_H

#include "../cli/common.h"

typedef struct {
    const char *name;
    size_t length;
    int index;
} Variable;

typedef struct {
    uint32_t cap;
    uint32_t count;
    Variable *variables;
} VariableArray;

void init_variable_array(VariableArray *variable_array);
void free_variable_array(VariableArray *variable_array);
uint32_t  write_variable(VariableArray *variable_array, Variable variable);
void write_variable_at(VariableArray *variable_array, uint8_t index, Variable variable);

#endif //CALC_VARIABLES_H
