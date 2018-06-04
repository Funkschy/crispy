#include "variables.h"
#include "../vm/memory.h"
#include "compiler.h"

void init_variable_array(VariableArray *variable_array) {
    variable_array->cap = 0;
    variable_array->count = 0;
    variable_array->variables = NULL;
}

void free_variable_array(VariableArray *variable_array) {
    FREE_ARR(variable_array->variables);
    init_variable_array(variable_array);
}

uint32_t write_variable(VariableArray *variable_array, Variable variable) {
    if(variable_array->count >= variable_array->cap) {
        variable_array->cap = GROW_CAP(variable_array->cap);
        variable_array->variables = GROW_ARR(variable_array->variables, Variable, variable_array->cap);
    }

    uint32_t count = variable_array->count;
    variable_array->variables[variable_array->count++] = variable;
    return count;
}
