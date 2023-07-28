#pragma once
#include "rt_slist.h"
#include "stdint.h"
#include "stdbool.h"

typedef struct
{
    rt_slist_t node;
    uint8_t    addr;
    uint8_t    step;
    uint32_t   offset;
} boot_update_list_t;

void clr_update_root(rt_slist_t *root);
bool add_update_node(rt_slist_t *root, uint8_t addr);
void set_update_node_process(rt_slist_t *root, uint8_t src_addr, uint8_t step, uint32_t offset);
bool judge_update_node_is_same(rt_slist_t *root, uint8_t step, uint32_t offset);
void remove_update_node_diff(rt_slist_t *root, uint8_t step, uint32_t offset);
bool update_node_is_exist(rt_slist_t *root, uint8_t addr);
void remove_update_node(rt_slist_t *root, uint8_t addr);
