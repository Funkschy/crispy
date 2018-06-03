#ifndef CALC_COMPILER_H
#define CALC_COMPILER_H

#include "scanner.h"
#include "variables.h"
#include "../vm/options.h"

typedef struct {
    Token token;
    Token previous;
    Scanner scanner;

    VariableArray variables[SCOPES_MAX];
    uint32_t scope_depth;

} Compiler;

#endif //CALC_COMPILER_H
