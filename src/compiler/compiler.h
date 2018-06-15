// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

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
    // TODO find alternative; causes wasted memory
    uint32_t vars_in_scope;

    HashTable natives;

    bool print_expr;
} Compiler;

#endif //CALC_COMPILER_H
