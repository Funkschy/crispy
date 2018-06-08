#include <stdio.h>

#include "debug.h"
#include "opcode.h"
#include "vm.h"

void disassemble_vm(Vm *vm, const char *name) {
    printf("======== %s ========\n", name);

    for (int i = 0; i < CURR_FRAME(vm).code_buffer.count;) {
        i = disassemble_instruction(vm, i);
    }
}

static int simple_instruction(const char *name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int constant_instruction(const char *name, Vm *vm, int offset) {
    uint8_t constant_index = CURR_FRAME(vm).code_buffer.code[offset + 1];

    printf("%-16s %4d '", name, constant_index);
    print_value(CURR_FRAME(vm).code_buffer.constants.values[constant_index], false);
    printf("'\n");

    return offset + 2;
}

static int var_instruction(const char *name, Vm *vm, int offset) {
    uint8_t var_index = CURR_FRAME(vm).code_buffer.code[offset + 1];

    printf("%-16s %4d\n", name, var_index);

    return offset + 2;
}

static int jump_instruction(const char *name, Vm *vm, int offset) {
    uint16_t jmp_address = (CURR_FRAME(vm).code_buffer.code[offset + 1] << 8) | CURR_FRAME(vm).code_buffer.code[offset + 2];

    printf("%-16s   -> %04d\n", name, jmp_address);

    return offset + 3;
}

int disassemble_instruction(Vm *vm, int offset) {
    printf("%04d ", offset);

    uint8_t instruction = CURR_FRAME(vm).code_buffer.code[offset];
    switch (instruction) {
        case OP_NOP:
            return simple_instruction("OP_NOP", offset);
        case OP_ADD:
            return simple_instruction("OP_ADD", offset);
        case OP_SUB:
            return simple_instruction("OP_SUB", offset);
        case OP_MUL:
            return simple_instruction("OP_MUL", offset);
        case OP_EQUAL:
            return simple_instruction("OP_EQUAL", offset);
        case OP_NOT_EQUAL:
            return simple_instruction("OP_NOT_EQUAL", offset);
        case OP_GREATER:
            return simple_instruction("OP_GREATER", offset);
        case OP_LESS:
            return simple_instruction("OP_LESS", offset);
        case OP_DIV:
            return simple_instruction("OP_DIV", offset);
        case OP_LDC:
            return constant_instruction("OP_LDC", vm, offset);
        case OP_LDC_W:
            return constant_instruction("OP_LDC_W", vm, offset);
        case OP_NEGATE:
            return simple_instruction("OP_NEGATE", offset);
        case OP_STORE:
            return var_instruction("OP_STORE", vm, offset);
        case OP_LOAD:
            return var_instruction("OP_LOAD", vm, offset);
        case OP_DUP:
            return simple_instruction("OP_DUP", offset);
        case OP_POP:
            return simple_instruction("OP_POP", offset);
        case OP_PRINT:
            return simple_instruction("OP_PRINT", offset);
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset);
        case OP_JMP:
            return jump_instruction("OP_JMP", vm, offset);
        case OP_JEQ:
            return jump_instruction("OP_JEQ", vm, offset);
        case OP_JMT:
            return jump_instruction("OP_JMT", vm, offset);
        case OP_JMF:
            return jump_instruction("OP_JMF", vm, offset);
        case OP_JNE:
            return jump_instruction("OP_JNE", vm, offset);
        case OP_JLT:
            return jump_instruction("OP_JLT", vm, offset);
        case OP_JLE:
            return jump_instruction("OP_JLE", vm, offset);
        case OP_JGT:
            return jump_instruction("OP_JGT", vm, offset);
        case OP_JGE:
            return jump_instruction("OP_JGE", vm, offset);
        case OP_INC:
            return var_instruction("OP_INC", vm, offset);
        case OP_INC_1:
            return simple_instruction("OP_INC_1", offset);
        case OP_DEC:
            return var_instruction("OP_DEC", vm, offset);
        case OP_LDC_0:
            return simple_instruction("OP_LDC_0", offset);
        case OP_LDC_1:
            return simple_instruction("OP_LDC_1", offset);
        case OP_CALL:
            return var_instruction("OP_CALL", vm, offset);
        case OP_CALL_NATIVE:
            return var_instruction("OP_CALL_NATIVE", vm, offset);
        case OP_MOD:
            return simple_instruction("OP_MOD", offset);
        case OP_TRUE:
            return simple_instruction("OP_TRUE", offset);
        case OP_FALSE:
            return simple_instruction("OP_FALSE", offset);
        case OP_NIL:
            return simple_instruction("OP_NIL", offset);
        default:
            printf("Unknown instruction %d\n", instruction);
            return offset + 1;
    }
}