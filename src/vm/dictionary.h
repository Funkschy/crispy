// Copyright (c) 2018 Felix Schoeller
// 
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef CRISPY_DICTIONARY_H
#define CRISPY_DICTIONARY_H

#include "hashtable.h"

typedef struct {
    Object object;

    HashTable content;
} ObjDict;

void print_dict(ObjDict *dict, bool new_line);

char *dict_to_string(ObjDict *dict);

#endif //CRISPY_DICTIONARY_H
