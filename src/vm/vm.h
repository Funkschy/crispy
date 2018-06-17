// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef VM_H
#define VM_H

#define CURR_FRAME(vm_ptr)          ((vm_ptr)->frames.frame_pointers[(vm_ptr)->frame_count - 1])
#define FRAME_AT(vm_ptr, offset)    ((vm_ptr)->frames.frame_pointers[(offset) - 1])
#define PUSH_FRAME(vm_ptr, frame)   (frames_write_at(&(vm_ptr)->frames, (vm_ptr)->frame_count++, frame))
#define POP_FRAME(vm_ptr)           ((vm_ptr)->frames.frame_pointers[--(vm_ptr)->frame_count])

#include "../cli/common.h"
#include "value.h"
#include "dictionary.h"
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

/**
 * Calls the garbage collector.
 * @param vm the current vm.
 */
void gc(Vm *vm);

/**
 * Initialises a Vm.
 * @param vm the vm.
 * @param interactive should the vm be started in interactive (shell) mode.
 */
void vm_init(Vm *vm, bool interactive);

/**
 * Frees a vm. Should be called for every initialised vm.
 * @param vm the vm
 */
void vm_free(Vm *vm);

/**
 * Initialises a frame array (inside the vm).
 * @param frames the framearray.
 */
void frames_init(FrameArray *frames);

/**
 * Frees a frame array.
 * @param frames the framearray.
 */
void frames_free(FrameArray *frames);

/**
 * Sets a frame in a framearray.
 * @param frame_arr the frame array.
 * @param index the index at which the frame will be inserted.
 * @param frame the frame.
 */
void frames_write_at(FrameArray *frame_arr, uint32_t index, CallFrame *frame);

/**
 * Compiles the source code inside the scanner of the compiler in the passed vm.
 * The generated bytecode is written inside the callframes of the vm.
 * @param vm the vm.
 * @return either 0 if the compilation was successful or INTERPRET_COMPILE_ERROR otherwise.
 */
int compile(Vm *vm);

/**
 * Frees an object.
 * @param object the object.
 * @return the size of the freed object.
 */
size_t free_object(Object *object);

/**
 * Adds a constant to the current callframes constant pool. 
 * @param vm the current vm.
 * @param value the constant.
 * @return the position of the inserted constant inside the pool. (Max 2**16)
 */
uint32_t add_constant(Vm *vm, Value value);

/**
 * Write an instruction to the codebuffer.
 * @param code_buffer the codebuffer.
 * @param instruction the instruction.
 */
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
 * Creates a new dictionary.
 * @param vm the current VM.
 * @param content the HashTable which represents the dictionary.
 * @return a pointer to the created dictionary.
 */
ObjDict *new_dict(Vm *vm, HashTable content);

/**
 * Compiles and executes the source code.
 * @param vm the VM to use for execution.
 * @param source the crispy source code.
 * @return the result of running the program (either ok, runtime error or compilation error).
 */
InterpretResult interpret(Vm *vm, const char *source);

/**
 * Compiles and executes the source code in interactive (shell) mode.
 * @param vm the current vm.
 * @param source the source code.
 * @return the result of running the program (either ok, runtime error or compilation error).
 */
InterpretResult interpret_interactive(Vm *vm, const char *source);

#endif