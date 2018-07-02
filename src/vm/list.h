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

/**
 * Appends an element to the end of a list.
 */
void list_append(ObjList *list, CrispyValue value);

/**
 * Set the element at an index to a value.
 * @param list the list.
 * @param index the index.
 * @param value the value.
 * @return true if successful else false.
 */
bool list_add(ObjList *list, int64_t index, CrispyValue value);

/**
 * Retrieve an element from a list.
 * @param list the list.
 * @param index the index.
 * @param return_value will be set to the return value if successful.
 * @return true if successful else false.
 */
bool list_get(ObjList *list, int64_t index, CrispyValue *return_value);

#endif //CRISPY_LIST_H
