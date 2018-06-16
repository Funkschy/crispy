// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef CALC_OPCODE_H
#define CALC_OPCODE_H

typedef enum {
    OP_NOP,

    OP_TRUE,
    OP_FALSE,
    OP_NIL,

    OP_ADD,             // add two numbers
    OP_SUB,             // subtract two numbers
    OP_MUL,             // multiply two numbers
    OP_DIV,             // divide two numbers
    OP_MOD,             // modulo operator

    OP_AND,
    OP_OR,

    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_GT,              // greater than
    OP_LT,              // less than
    OP_GE,              // greater equals
    OP_LE,              // less equals

    OP_LDC,             // push number from constant pool to stack
    OP_LDC_W,           // push number from constant pool to stack index: (indexbyte1 << 8) | indexbyte2
    OP_LDC_0,           // Load 0 as a constant
    OP_LDC_1,           // Load 1 as a constant

    OP_STORE,           // store number as variable
    OP_LOAD,            // load variable to stack
    OP_LOAD_OFFSET,     // load a variable from a different callframe
    OP_STORE_OFFSET,    // store a variable in a different callframe
    OP_DUP,             // duplicate
    OP_POP,             // pop from stack

    OP_CALL,            // pop a function from the stack and call it
    OP_NEGATE,          // negate a number
    OP_NOT,             // reverse the boolean on top of the stack

    OP_PRINT,           // pops a value from the stack and prints it. (Only used in interactive mode)

    OP_DICT_ADD,        // pops to values from the stack and adds them to a dictionary as a key-value pair
    OP_DICT_GET,        // pops key from stack and tries to retrieve a value from the dict on top of the stack

    OP_JMP,             // unconditional jump
    OP_JEQ,             // jump if equals
    OP_JMT,             // jump if true
    OP_JMF,             // jump if false
    OP_JNE,             // jump if not equals
    OP_JLT,             // jump if less than
    OP_JLE,             // jump if equals or less
    OP_JGT,             // jump if equals or greater
    OP_JGE,             // jump if greater

    OP_INC_1,           // increment by 1
    OP_DEC_1,           // decrement by 1

    OP_RETURN           // return from Scope
} OP_CODE;

#endif //CALC_OPCODE_H
