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

#ifndef VM_H
#define VM_H

#define CURR_FRAME(vm_ptr)          ((vm_ptr)->frames.frame_pointers[(vm_ptr)->frame_count - 1])
#define FRAME_AT(vm_ptr, offset)    ((vm_ptr)->frames.frame_pointers[(offset) - 1])
#define PUSH_FRAME(vm_ptr, frame)   (frames_write_at(&(vm_ptr)->frames, (vm_ptr)->frame_count++, frame))
#define POP_FRAME(vm_ptr)           ((vm_ptr)->frames.frame_pointers[--(vm_ptr)->frame_count])

#include "../cli/common.h"
#include "value.h"
#include "../compiler/compiler.h"
#include "options.h"

typedef enum {
    INTERPRET_OK,
    INTERPRET_RUNTIME_ERROR,
    INTERPRET_COMPILE_ERROR
} InterpretResult;

typedef struct {
    uint32_t count;
    uint32_t cap;
    CallFrame **frame_pointers;
} FrameArray;

typedef struct {
    Value stack[STACK_MAX];
    Value *sp;

    FrameArray frames;
    uint32_t frame_count;

    Compiler compiler;

    HashTable strings;

    size_t allocated_mem;
    size_t max_alloc_mem;
    Object *first_object;

    // indicates if the program
    // is running in shell mode
    bool interactive;
} Vm;

void gc(Vm *vm);

void init_vm(Vm *vm, bool interactive);

void free_vm(Vm *vm);

void frames_init(FrameArray *frames);

void frames_free(FrameArray *frames);

void frames_write_at(FrameArray *frame_arr, uint32_t index, CallFrame *frame);

void compile(Vm *vm);

size_t free_object(Object *object);

uint32_t add_constant(Vm *vm, Value value);

void write_code_buffer(CodeBuffer *code_buffer, uint8_t instruction);

/**
 * Allocate a new lambda Object.
 * @param vm the current VM.
 * @param num_params the number of arguments, the lambda expects.
 * @return a pointer to the created lambda.
 */
ObjLambda *new_lambda(Vm *vm, uint8_t num_params);

/**
 * Creates an empty string.
 * @param vm the current VM.
 * @param length the length of the new string.
 * @return a pointer to the created string.
 */
ObjString *new_empty_string(Vm *vm, size_t length);

/**
 * Creates a string and copies the values from start until (start + length) into it.
 * @param vm the current VM.
 * @param start a pointer to the first char of the string
 * @param length the length of the new string.
 * @return a pointer to the created string.
 */
ObjString *new_string(Vm *vm, const char *start, size_t length);

/**
 * Allocate a new native function Object.
 * @param vm the current VM.
 * @param func_ptr a pointer to the c function.
 * @param num_params the number of expected parameters.
 * @param system_func Determines, if the function belongs to the standard library,
 * if true, the current vm will be passed to the function.
 * @return a pointer to the created function wrapper.
 */
ObjNativeFunc *new_native_func(Vm *vm, void *func_ptr, uint8_t num_params, bool system_func);

/**
 * Compiles and executes the source code.
 * @param vm the VM to use for execution.
 * @param source the crispy source code.
 * @return the result of running the program (either ok, runtime error or compilation error).
 */
InterpretResult interpret(Vm *vm, const char *source);

InterpretResult interpret_interactive(Vm *vm, const char *source);

#endif