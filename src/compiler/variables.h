// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef CALC_VARIABLES_H
#define CALC_VARIABLES_H

#include "../util/common.h"

typedef struct {
    int index;
	int scope;
    int frame_offset;
    bool assignable;
} Variable;

typedef struct {
    const char *key_ident_string;
    size_t ident_length;
} VarHTItemKey;

typedef struct var_table_item_t {
    VarHTItemKey key;
    Variable value;
    struct var_table_item_t *next;
} VarHTItem;

typedef struct {
    uint32_t cap;
    uint32_t size;

    VarHTItem **buckets;
} VarHashTable;

uint32_t var_hash(VarHTItemKey key);

void var_ht_init(VarHashTable *ht, uint32_t init_cap);

void var_ht_free(VarHashTable *ht);

Variable *var_ht_get(VarHashTable *ht, VarHTItemKey key);

void var_ht_delete(VarHashTable *ht, VarHTItemKey key);

void var_ht_put(VarHashTable *ht, VarHTItemKey key, Variable value);

#endif //CALC_VARIABLES_H
