// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdlib.h>
#include <string.h>

#include "variables.h"
#include "../vm/memory.h"
#include "compiler.h"
#include "../vm/value.h"

uint32_t var_hash(VarHTItemKey key) {
    return hash_string(key.key_ident_string, key.ident_length);
}

void var_ht_init(VarHashTable *ht, uint32_t init_cap) {
    ht->cap = (uint32_t) next_pow_of_2(init_cap);
    ht->size = 0;
    ht->buckets = NULL;

    ht->buckets = calloc(ht->cap, sizeof(VarHTItem *));
}

static void free_bucket(VarHTItem *bucket) {
    VarHTItem *item = bucket;
    while (item) {
        VarHTItem *next = item->next;
        free(item);
        item = next;
    }
}

void var_ht_free(VarHashTable *ht) {
    for (int i = 0; i < ht->cap; ++i) {
        free_bucket(ht->buckets[i]);
    }
    free(ht->buckets);
}

static bool equals(VarHTItemKey first, VarHTItemKey second) {
    if (first.ident_length != second.ident_length) {
        return false;
    }
    return memcmp(first.key_ident_string, second.key_ident_string, first.ident_length) == 0;
}

/**
 * Insert an item into a bucket.
 * @param bucket the bucket.
 * @param key the key.
 * @param type the key type.
 * @param new_item the item to insert.
 * @return true if the key already was in the map, false if it had to be created.
 */
static bool insert(VarHTItem **bucket, VarHTItemKey key, VarHTItem *new_item) {
    if (equals((*bucket)->key, key)) {
        VarHTItem *next = (*bucket)->next;
        free(*bucket);
        *bucket = new_item;
        (*bucket)->next = next;
        return true;
    }

    VarHTItem *previous = *bucket;
    VarHTItem **current = &(*bucket)->next;

    while (*current) {
        if (equals((*current)->key, key)) {
            VarHTItem *next = (*current)->next;
            free(*current);
            *current = new_item;
            (*current)->next = next;
            return true;
        }

        previous = *current;
        current = &(*current)->next;
    }

    previous->next = new_item;
    return false;
}

static void resize(VarHashTable *ht) {
    VarHashTable new_ht;
    var_ht_init(&new_ht, (uint32_t) next_pow_of_2(ht->cap + 1));

    for (uint32_t i = 0; i < ht->cap; ++i) {
        VarHTItem *item = ht->buckets[i];

        while (item) {
            var_ht_put(&new_ht, item->key, item->value);
            item = item->next;
        }
    }

    var_ht_free(ht);
    *ht = new_ht;
}

static Variable *find(VarHTItem *bucket, VarHTItemKey wanted) {
    VarHTItem *item = bucket;
    Variable *value = NULL;

    while (item) {
        if (equals(item->key, wanted)) {
            return &item->value;
        }

        item = item->next;
    }

    return value;
}

Variable *var_ht_get(VarHashTable *ht, VarHTItemKey key) {
    uint32_t index = var_hash(key) & (ht->cap - 1);

    if (ht->buckets[index] != NULL) {
        Variable *value = find(ht->buckets[index], key);
        if (value) {
            return value;
        }
    }

    return NULL;
}

void var_ht_delete(VarHashTable *ht, VarHTItemKey key) {

}

void var_ht_put(VarHashTable *ht, VarHTItemKey key, Variable value) {
    uint32_t index = var_hash(key) & (ht->cap - 1);
    VarHTItem *new_item = malloc(sizeof(VarHTItem));
    new_item->next = NULL;
    new_item->key = key;
    new_item->value = value;

    bool already_inside = false;
    if (ht->buckets[index] == NULL) {
        ht->buckets[index] = new_item;
    } else {
        already_inside = insert(&ht->buckets[index], key, new_item);
    }

    if (!already_inside) {
        ++ht->size;
    }

    if (ht->size > ht->cap) {
        resize(ht);
    }
}
