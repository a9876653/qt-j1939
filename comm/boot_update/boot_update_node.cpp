#include "boot_update_node.h"
#include "stdio.h"
#include "stdlib.h"
#include "middle_signal.h"

#define BOOT_NODE_DBG(x...) MiddleSignalIns->dbg_info(x)

void clr_update_root(rt_slist_t *root)
{
    rt_slist_t *next        = NULL;
    rt_slist_t *update_node = root->next;
    while (update_node)
    {
        next = update_node->next;

        void *entry = rt_slist_entry(update_node, boot_update_list_t, node);
        free(entry);
        update_node = next;
    }
    root->next = NULL;
    BOOT_NODE_DBG("boot update clr all node");
}

bool add_update_node(rt_slist_t *root, uint8_t addr)
{
    boot_update_list_t *update_node = (boot_update_list_t *)malloc(sizeof(boot_update_list_t));
    if (update_node != NULL)
    {
        update_node->addr   = addr;
        update_node->offset = 0;
        update_node->step   = 0;

        rt_slist_append(root, &(update_node->node));
        return true;
    }
    BOOT_NODE_DBG("boot update add node failed");
    return false;
}

void set_update_node_process(rt_slist_t *root, uint8_t src_addr, uint8_t step, uint32_t offset)
{
    boot_update_list_t *update_node = NULL;

    /** 遍历单链表 update_list */
    rt_slist_type_for_each_entry(boot_update_list_t, update_node, root, node)
    {
        if (update_node->addr == src_addr)
        {
            update_node->step   = step;
            update_node->offset = offset;
            return;
        }
    }
}

bool judge_update_node_is_same(rt_slist_t *root, uint8_t step, uint32_t offset)
{
    boot_update_list_t *update_node = NULL;

    /** 遍历单链表 update_list */
    rt_slist_type_for_each_entry(boot_update_list_t, update_node, root, node)
    {
        if (update_node->step != step || update_node->offset != offset)
        {
            return false;
        }
    }
    return true;
}

void remove_update_node_diff(rt_slist_t *root, uint8_t step, uint32_t offset)
{
    rt_slist_t *next        = NULL;
    rt_slist_t *update_node = root->next;
    while (update_node)
    {
        next = update_node->next;

        boot_update_list_t *entry = rt_slist_entry(update_node, boot_update_list_t, node);
        if (entry->step != step || entry->offset != offset)
        {
            rt_slist_remove(root, update_node);
            BOOT_NODE_DBG("boot update remove node, node->addr %d, node->step %d, node->offset %d, step %d, offset %d",
                          entry->addr,
                          entry->step,
                          entry->offset,
                          step,
                          offset);
            free(entry);
        }
        update_node = next;
    }
}
