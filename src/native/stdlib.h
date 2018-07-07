// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef CRISPY_STDLIB_H
#define CRISPY_STDLIB_H

#include "../vm/value.h"
#include "../vm/vm.h"

CrispyValue std_println(CrispyValue *value);

CrispyValue std_print(CrispyValue *value);

CrispyValue std_exit(CrispyValue *value, Vm *vm);

CrispyValue std_str(CrispyValue *value, Vm *vm);

CrispyValue std_len(CrispyValue *value, Vm *vm);

CrispyValue std_split(CrispyValue *value, Vm *vm);

CrispyValue std_input(CrispyValue *value, Vm *vm);

CrispyValue std_list(CrispyValue *value, Vm *vm);

CrispyValue std_num(CrispyValue *value, Vm *vm);

#endif //CRISPY_STDLIB_H
