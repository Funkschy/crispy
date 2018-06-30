// Copyright (c) 2018 Felix Schoeller
// 
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include "list.h"

void list_add(ObjList *list, CrispyValue value) {
    write_value(&list->content, value);
}