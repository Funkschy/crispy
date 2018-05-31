#include <stdio.h>

#include "debug.h"
#include "opcode.h"
#include "chunk.h"

void disassemble_chunk(Chunk *chunk, const char *name) {
    printf("======== %s ========\n", name);

    for (int i = 0; i < chunk->count;) {
        i = disassemble_instruction(chunk, i);
    }
}

static int simple_instruction(const char *name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int constant_instruction(const char *name, Chunk *chunk, int offset) {
    uint8_t constant_index = chunk->code[offset + 1];

    printf("%-16s %4d '", name, constant_index);
    print_value(chunk->constants.values[constant_index]);
    printf("'\n");

    return offset + 2;
}

static int var_instruction(const char *name, Chunk *chunk, int offset) {
    uint8_t var_index = chunk->code[offset + 1];

    printf("%-16s %4d\n", name, var_index);

    return offset + 2;
}

static int jump_instruction(const char *name, Chunk *chunk, int offset) {
    uint16_t jmp_address = (chunk->code[offset + 1] << 8) | chunk->code[offset + 2];

    printf("%-16s   -> %04d\n", name, jmp_address);

    return offset + 3;
}

int disassemble_instruction(Chunk *chunk, int offset) {
    printf("%04d ", offset);

    uint8_t instruction = chunk->code[offset];
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
            return constant_instruction("OP_LDC", chunk, offset);
        case OP_LDC_W:
            return constant_instruction("OP_LDC_W", chunk, offset);
        case OP_NEGATE:
            return simple_instruction("OP_NEGATE", offset);
        case OP_STORE:
            return var_instruction("OP_STORE", chunk, offset);
        case OP_LOAD:
            return var_instruction("OP_LOAD", chunk, offset);
        case OP_DUP:
            return simple_instruction("OP_DUP", offset);
        case OP_POP:
            return simple_instruction("OP_POP", offset);
        case OP_PRINT:
            return simple_instruction("OP_PRINT", offset);
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset);
        case OP_JMP:
            return jump_instruction("OP_JMP", chunk, offset);
        case OP_JEQ:
            return jump_instruction("OP_JEQ", chunk, offset);
        case OP_JMT:
            return jump_instruction("OP_JMT", chunk, offset);
        case OP_JMF:
            return jump_instruction("OP_JMF", chunk, offset);
        case OP_JNE:
            return jump_instruction("OP_JNE", chunk, offset);
        case OP_JLT:
            return jump_instruction("OP_JLT", chunk, offset);
        case OP_JLE:
            return jump_instruction("OP_JLE", chunk, offset);
        case OP_JGT:
            return jump_instruction("OP_JGT", chunk, offset);
        case OP_JGE:
            return jump_instruction("OP_JGE", chunk, offset);
        case OP_INC:
            return var_instruction("OP_INC", chunk, offset);
        case OP_INC_1:
            return simple_instruction("OP_INC_1", offset);
        case OP_DEC:
            return var_instruction("OP_DEC", chunk, offset);
        case OP_LDC_0:
            return simple_instruction("LDC_0", offset);
        case OP_LDC_1:
            return simple_instruction("LDC_1", offset);
        default:
            printf("Unknown instruction %d\n", instruction);
            return offset + 1;
    }
}