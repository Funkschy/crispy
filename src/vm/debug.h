// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef DEBUG_H
#define DEBUG_H

#include "vm.h"

void disassemble_curr_frame(Vm *vm, const char *name);
int disassemble_instruction(Vm *chunk, int offset);

#endif