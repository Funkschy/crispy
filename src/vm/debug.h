#ifndef DEBUG_H
#define DEBUG_H

#include "vm.h"

void disassemble_curr_frame(Vm *vm, const char *name);
int disassemble_instruction(Vm *chunk, int offset);

#endif