/* SPDX-License-Identifier: Apache-2.0 */

#include "hasht.h"

#define KEY_UNDEF_VAL (0x1u << 31)
#define KEY_MASK(_k)  ((_k)&0x7FFFFFFFu) /* 30 bits keys */

static inline uint32_t next_hash(hasht_t *ht, const uint32_t hash)
{
    return (hash + 1u) % ht->max_size;
}

static inline uint32_t hash_code(hasht_t *ht, const uint32_t key)
{
    return key % ht->max_size;
}

void hasht_clear(hasht_t *ht)
{
    ht->size = 0;
}

int hasht_delete(hasht_t *ht, const uint32_t key)
{
    size_t         nitems = ht->size;
    const uint32_t k      = KEY_MASK(key);
    uint32_t       hash   = hash_code(ht, k);

    if (ht->size > 0)
    {
        while (nitems-- != 0)
        {
            if (ht->items[hash].key == k)
            {
                ht->items[hash].item = NULL;
                ht->items[hash].key  = KEY_UNDEF_VAL;
                ht->size--;
                return 0;
            }
            hash = next_hash(ht, hash);
        }
    }
    return -EHASHT_NFOUND;
}

hasht_entry_t *hasht_search(hasht_t *ht, const uint32_t key)
{
    const uint32_t k      = KEY_MASK(key);
    uint32_t       hash   = hash_code(ht, k);
    size_t         nitems = ht->size;
    while (nitems-- != 0)
    {
        if (ht->items[hash].key == k)
        {
            return &ht->items[hash];
        }
        hash = next_hash(ht, hash);
    }
    return NULL;
}

int hasht_insert(hasht_t *ht, const uint32_t key, void *data)
{
    const uint32_t k    = KEY_MASK(key);
    uint32_t       hash = hash_code(ht, k);

    if (ht->size == ht->max_size)
    {
        return -EHASHT_FULL;
    }

    if (hasht_search(ht, key) == NULL)
    {
        while (ht->items[hash].item != NULL)
        {
            hash = next_hash(ht, hash);
        }
        ht->items[hash].key  = k;
        ht->items[hash].item = data;
        ht->size++;
        return key;
    }
    return -EHASHT_DUP;
}

void hasht_init(hasht_t *ht)
{
    for (size_t i = 0; i < ht->max_size; i++)
    {
        ht->items[i].key = KEY_UNDEF_VAL;
    }
}