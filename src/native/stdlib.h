// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef CRISPY_STDLIB_H
#define CRISPY_STDLIB_H

#include "../vm/value.h"
#include "../vm/vm.h"

Value println(Value *value);

Value print(Value *value);

Value exit_vm(Value *value);

Value str(Value *value, Vm *vm);

#endif //CRISPY_STDLIB_H
