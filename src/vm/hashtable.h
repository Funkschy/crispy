#ifndef CRISPY_HASHMAP_H
#define CRISPY_HASHMAP_H

#include "value.h"

typedef enum {
    HT_KEY_OBJSTRING, HT_KEY_CSTRING, HT_KEY_INT
} HTKeyType;

typedef union {
    const char *key_c_string;
    ObjString *key_obj_string;
    uint32_t key_int;
} HTItemKey;

typedef struct table_item_t {
    HTItemKey key;
    Value value;
    struct table_item_t *next;
} HTItem;

typedef struct {
    HTKeyType key_type;

    // TODO use
    float load_factor;
    uint32_t cap;
    uint32_t size;

    HTItem **buckets;
} HashTable;

uint32_t hash(HTItemKey key, HTKeyType type);

void ht_init(HashTable *ht, HTKeyType key_type, uint32_t init_cap, float load_factor);

void ht_free(HashTable *ht);

Value ht_get(HashTable *ht, HTItemKey key);

void ht_delete(HashTable *ht, HTItemKey key);

void ht_put(HashTable *ht, HTItemKey key, Value value);

#endif //CRISPY_HASHMAP_H
