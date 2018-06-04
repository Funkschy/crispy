#ifndef CALC_COMPILER_H
#define CALC_COMPILER_H

#include "scanner.h"
#include "variables.h"
#include "../vm/options.h"

typedef struct {
    Token token;
    Token previous;
    Scanner scanner;

    VariableArray scope[SCOPES_MAX];
    uint32_t scope_depth;
    uint32_t vars_in_scope;
} Compiler;

#endif //CALC_COMPILER_H
