// Copyright (c) 2018 Felix Schoeller
// 
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "dictionary.h"
#include "hashtable.h"
#include "value.h"
#include "memory.h"


static size_t ht_key_to_string(HTKeyType type, HTItemKey key, char **dest) {
    switch (type) {
        case HT_KEY_CSTRING:
            *dest = strdup(key.key_c_string);
            return strlen(key.key_c_string);
        case HT_KEY_OBJSTRING: {
            size_t size = (key.key_obj_string->length + 1) * sizeof(char);
            char *string = malloc(size);
            memcpy(string, key.key_obj_string->start, key.key_obj_string->length);
            string[size - 1] = '\0';
            *dest = string;
            return key.key_obj_string->length;
        }
        case HT_KEY_INT: {
            // all numbers represented by a 32 bit unsigned integer fit into 10 bytes
            char *string = malloc(11 * sizeof(char));
            sprintf(string, "%d", key.key_int);
            *dest = string;
            return strlen(string);
        }
        case HT_KEY_IDENT_STRING: {
            size_t size = (key.ident_length + 1) * sizeof(char);
            char *string = malloc(size);
            memcpy(string, key.key_ident_string, key.ident_length);
            string[size - 1] = '\0';
            return key.ident_length;
        }
    }

    return 0;
}

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

typedef struct {
    char *start;
    size_t length;
} String;

typedef struct {
    uint32_t count;
    uint32_t cap;
    String *strings;
} StringArray;

static void init_string_array(StringArray *string_array) {
    string_array->cap = 0;
    string_array->count = 0;
    string_array->strings = NULL;
}

static void free_string_array(StringArray *string_array) {
    FREE_ARR(string_array->strings);
    init_string_array(string_array);
}

static void write_string(StringArray *string_array, char *string, size_t length) {
    if (string_array->count >= string_array->cap) {
        string_array->cap = GROW_CAP(string_array->cap);
        string_array->strings = GROW_ARR(string_array->strings, String, string_array->cap);
    }

    string_array->strings[string_array->count].start = string;
    string_array->strings[string_array->count].length = length;
    ++string_array->count;
}

char *dict_to_string(ObjDict *dict) {
    HashTable ht = dict->content;
    StringArray str_arr;
    init_string_array(&str_arr);

    size_t size = 0;
    size_t temp_size;

    bool first = true;

    for (uint32_t i = 0; i < ht.cap; ++i) {
        HTItem *bucket = ht.buckets[i];
        while (bucket) {
            if (first) {
                first = false;
                write_string(&str_arr, strdup("\""), 1);
                size += 1;
            } else {
                write_string(&str_arr, strdup(", \""), 3);
                size += 3;
            }

            char *key;
            temp_size = ht_key_to_string(ht.key_type, bucket->key, &key);
            write_string(&str_arr, key, temp_size);
            size += temp_size;

            size += 3;
            write_string(&str_arr, strdup("\": "), 3);

            char *val;
            temp_size = value_to_string(bucket->value, &val);
            write_string(&str_arr, val, temp_size);
            size += temp_size;

            bucket = bucket->next;
        }
    }

    char *string = malloc((size + 3)* sizeof(char));
    size_t offset = 1;

    for (uint32_t j = 0; j < str_arr.count; ++j) {
        String *s = &str_arr.strings[j];
        memcpy(string + offset, s->start, s->length);
        free(str_arr.strings[j].start);
        offset += s->length;
    }

    string[0] = '{';
    string[size + 1] = '}';
    string[size + 2] = '\0';

    free_string_array(&str_arr);

    return string;
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