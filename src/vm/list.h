// Copyright (c) 2018 Felix Schoeller
// 
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef CRISPY_LIST_H
#define CRISPY_LIST_H

#include "value.h"

typedef struct {
    Object object;

    ValueArray content;
} ObjList;

void list_add(ObjList *list, CrispyValue value);

#endif //CRISPY_LIST_H
