/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __HASHT_H__
#define __HASHT_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define EHASHT_EMPTY  1
#define EHASHT_FULL   2
#define EHASHT_NFOUND 3
#define EHASHT_DUP    4

#define HASHT_INIT(_items, maxsize)                      \
    {                                                    \
        .items = _items, .max_size = maxsize, .size = 0, \
    }

typedef struct
{
    uint32_t key;
    void    *item;
} hasht_entry_t;

typedef struct
{
    hasht_entry_t *items;
    size_t         max_size;
    size_t         size;
} hasht_t;

int            hasht_insert(hasht_t *ht, const uint32_t key, void *data);
hasht_entry_t *hasht_search(hasht_t *ht, const uint32_t key);
int            hasht_delete(hasht_t *ht, const uint32_t key);
void           hasht_clear(hasht_t *ht);
void           hasht_init(hasht_t *ht);

#endif /* __HASHT_H__ */
