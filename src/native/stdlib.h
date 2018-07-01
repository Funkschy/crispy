// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef CRISPY_STDLIB_H
#define CRISPY_STDLIB_H

#include "../vm/value.h"
#include "../vm/vm.h"

CrispyValue println(CrispyValue *value);

CrispyValue print(CrispyValue *value);

CrispyValue exit_vm(CrispyValue *value, Vm *vm);

CrispyValue str(CrispyValue *value, Vm *vm);

CrispyValue len(CrispyValue *value, Vm *vm);

#endif //CRISPY_STDLIB_H
