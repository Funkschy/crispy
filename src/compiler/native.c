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

#include "native.h"

#include "../native/stdlib.h"

static void make_native(Vm *vm, const char *name, void *fn_ptr, uint8_t num_params, bool pass_vm) {
    ObjNativeFunc *println = new_native_func(vm, fn_ptr, num_params, pass_vm);
    Value value = create_object((Object *) println);

    HTItemKey key;
    key.key_c_string = name;
    ht_put(&vm->compiler.natives, key, value);
}

void declare_natives(Vm *vm) {
    make_native(vm, "println", println, 1, false);
    make_native(vm, "print", print, 1, false);
    make_native(vm, "exit", exit_vm, 1, false);
    make_native(vm, "str", str, 1, true);
}
