// Copyright (c) 2018 Felix Schoeller
// 
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>

#include "dictionary.h"
#include "hashtable.h"

static void print_ht_key(HTKeyType type, HTItemKey key) {
    switch (type) {
        case HT_KEY_CSTRING:
            printf("%s", key.key_c_string);
            break;
        case HT_KEY_OBJSTRING:
            printf("%.*s", (int) key.key_obj_string->length, key.key_obj_string->start);
            break;
        case HT_KEY_INT:
            printf("%d", key.key_int);
            break;
        case HT_KEY_IDENT_STRING:
            printf("%.*s", (int) key.ident_length, key.key_ident_string);
            break;
    }
}

void print_dict(ObjDict *dict, bool new_line) {
    const char *nl = (new_line) ? "\n" : "";
    const char *tab = (new_line) ? "\t" : "";

    HashTable ht = dict->content;

    printf("{%s", nl);
    bool first = true;

    for (uint32_t i = 0; i < ht.cap; ++i) {
        HTItem *bucket = ht.buckets[i];
        while (bucket) {
            if (first) {
                first = false;
            } else {
                printf(", ");
            }
            printf("%s\"", tab);
            print_ht_key(ht.key_type, bucket->key);
            printf("\": ");
            print_value(bucket->value, new_line, true);

            bucket = bucket->next;
        }
    }

    printf("}%s", nl);
}