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

#include <stdio.h>
#include "stdlib.h"
#include "../vm/value.h"

Value println(Value *value) {
    print_value(value[0], true);

    return create_nil();
}

Value print(Value *value) {
    print_value(value[0], false);

    return create_nil();
}

// TODO maybe free vm?
Value exit_vm(Value *value) {
    if (value[0].type == NUMBER) {
        int ret_val = (int) value[0].d_value;
        exit(ret_val);
    }

    exit(1);
}

Value str(Value *value, Vm *vm) {
    char s[17];
    snprintf(s, 17, "%.15g", value->d_value);

    // one less, because crispy strings are not null terminated
    return create_object((Object *) new_string(vm, s, 16));
}