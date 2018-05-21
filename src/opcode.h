#ifndef CALC_OPCODE_H
#define CALC_OPCODE_H

typedef enum {
    OP_NOP,
    OP_ADD,         // add two numbers
    OP_SUB,         // subtract two numbers
    OP_MUL,         // multiply two numbers
    OP_DIV,         // divide two numbers
    OP_LDC,         // push number from constant pool to stack
    OP_LDC_W,       // push number from constant pool to stack index: (indexbyte1 << 8) | indexbyte2
    OP_LDC_0,       // Load 0 as a constant
    OP_LDC_1,       // Load 1 as a constant
    OP_NEGATE,      // negate a number
    OP_STORE,       // store number as variable
    OP_LOAD,        // load variable to stack
    OP_DUP,         // duplicate
    OP_PRINT,       // print
    OP_POP,         // pop from stack
    OP_JMP,         // unconditional jump
    OP_JEQ,         // jump if equals
    OP_JNE,         // jump if not equals
    OP_JLT,         // jump if less than
    OP_JLE,         // jump if equals or less
    OP_JGT,         // jump if equals or greater
    OP_JGE,         // jump if greater
    OP_INC,         // increment
    OP_INC_1,       // increment by 1
    OP_DEC,         // decrement
    OP_RETURN       // return from Chunk
} OP_CODE;

#endif //CALC_OPCODE_H
