#ifndef CALC_COMPILER_H
#define CALC_COMPILER_H

#include "scanner.h"
#include "variables.h"
#include "../vm/options.h"
#include "../vm/hashtable.h"

typedef struct {
    Token token;
    Token previous;
    Scanner scanner;

    VariableArray scope[SCOPES_MAX];
    uint32_t scope_depth;

    HashTable natives;
} Compiler;

#endif //CALC_COMPILER_H
