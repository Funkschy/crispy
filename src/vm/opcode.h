// Copyright (c) 2018 Felix Schoeller
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
