// Copyright (c) 2018 Felix Schoeller
// 
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include "list.h"
#include "value.h"

void list_append(ObjList *list, CrispyValue value) {
    write_value(&list->content, value);
}

bool list_add(ObjList *list, int64_t index, CrispyValue value) {
    if (index < 0 || index >= list->content.count) {
        return false;
    }

    write_at(&list->content, (uint64_t) index, value);
    return true;
}

bool list_get(ObjList *list, int64_t index, CrispyValue *return_value) {
    if (index < 0 || index >= list->content.count) {
        return false;
    }

    *return_value = list->content.values[index];
    return true;
}