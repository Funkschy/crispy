#ifndef DEBUG_H
#define DEBUG_H

#include "vm.h"

void disassemble_vm(Vm *vm, const char *name);
int disassemble_instruction(Vm *chunk, int offset);

#endif