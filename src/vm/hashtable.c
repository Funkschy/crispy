#include "hashtable.h"
#include "value.h"

#include <string.h>

uint32_t hash(HTItemKey key, HTKeyType type) {
    switch (type) {
        case HT_KEY_CSTRING:
            return hash_string(key.key_c_string, strlen(key.key_c_string));
        case HT_KEY_OBJSTRING:
            return hash_string_obj(key.key_obj_string);
        case HT_KEY_INT:
            return hash_uint32_t(key.key_int);
        case HT_KEY_IDENT_STRING:
            return hash_string(key.key_ident_string, key.ident_length);
    }
}

static bool equals(HTItemKey first, HTItemKey second, HTKeyType type) {
    switch (type) {
        case HT_KEY_CSTRING:
            return strcmp(first.key_c_string, second.key_c_string) == 0;
        case HT_KEY_OBJSTRING:
            if (first.key_obj_string->length != second.key_obj_string->length) {
                return false;
            }
            return cmp_strings(first.key_obj_string, second.key_obj_string) == 0;
        case HT_KEY_INT:
            return first.key_int == second.key_int;
        case HT_KEY_IDENT_STRING:
            if (first.ident_length != second.ident_length) {
                return false;
            }
            return memcmp(first.key_ident_string, second.key_ident_string, first.ident_length) == 0;
    }

    return false;
}

static uint32_t next_pow_of_2(uint32_t num) {
    uint32_t n = num - 1;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    // if n is 0 add 1
    n += 1 + (n == 0);
    return n;
}

void ht_init(HashTable *ht, HTKeyType key_type, uint32_t init_cap, void(*free_callback)(HTItem *)) {
    ht->cap = next_pow_of_2(init_cap);
    ht->size = 0;
    ht->buckets = NULL;
    ht->key_type = key_type;
    ht->free_callback = free_callback;

    ht->buckets = calloc(ht->cap, sizeof(HTItem *));
}

static void free_bucket(HTItem *bucket, void(free_callback)(HTItem *)) {
    HTItem *item = bucket;
    while (item) {
        HTItem *next = item->next;
        free_callback(item);
        item = next;
    }
}

void ht_free(HashTable *ht) {
    for (int i = 0; i < ht->cap; ++i) {
        free_bucket(ht->buckets[i], ht->free_callback);
    }
    free(ht->buckets);
}

static void insert(HTItem **bucket, HTItemKey key, HTKeyType type, HTItem *new_item) {
    if (equals((*bucket)->key, key, type)) {
        free(*bucket);
        *bucket = new_item;
        return;
    }

    HTItem *previous = *bucket;
    HTItem **current = &(*bucket)->next;

    while (*current) {
        if (equals((*current)->key, key, type)) {
            free(*current);
            *current = new_item;
            return;
        }

        previous = *current;
        current = &(*current)->next;
    }

    previous->next = new_item;
}

static void resize(HashTable *ht) {
    HashTable new_ht;
    ht_init(&new_ht, ht->key_type, next_pow_of_2(ht->cap + 1), ht->free_callback);

    for (uint32_t i = 0; i < ht->cap; ++i) {
        HTItem *item = ht->buckets[i];

        while (item) {
            ht_put(&new_ht, item->key, item->value);
            item = item->next;
        }
    }

    ht_free(ht);
    *ht = new_ht;
}

void ht_put(HashTable *ht, HTItemKey key, Value value) {
    uint32_t index = hash(key, ht->key_type) & (ht->cap - 1);
    HTItem *new_item = malloc(sizeof(HTItem));
    new_item->next = NULL;
    new_item->key = key;
    new_item->value = value;

    if (ht->buckets[index] == NULL) {
        ht->buckets[index] = new_item;
    } else {
        insert(&ht->buckets[index], key, ht->key_type, new_item);
    }

    if (++ht->size > ht->cap) {
        resize(ht);
    }
}

static Value *find(HTItem *bucket, HTItemKey wanted, HTKeyType type) {
    HTItem *item = bucket;
    Value *value = NULL;

    while (item) {
        if (equals(item->key, wanted, type)) {
            return &item->value;
        }

        item = item->next;
    }

    return value;
}

Value ht_get(HashTable *ht, HTItemKey key) {
    uint32_t index = hash(key, ht->key_type) & (ht->cap - 1);

    if (ht->buckets[index] != NULL) {
        Value *value = find(ht->buckets[index], key, ht->key_type);
        if (value) {
            return *value;
        }
    }

    Value v = {NIL, 0};
    return v;
}

void free_string_literal(HTItem *item) {
    free(item);
}

void free_heap_string(HTItem *item) {
    const char *string = item->key.key_c_string;
    free((void *) string);
    free(item);
}



