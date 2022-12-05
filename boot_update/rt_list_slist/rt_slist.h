#pragma once

#include "rt_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Single List structure
 * 单向链表节点
 */
typedef struct rt_slist_node
{
    struct rt_slist_node *next; /**< point to next node. */
} rt_slist_t;

/**
 * @brief initialize a list object
 * 初始化链表对象
 */
#define RT_SLIST_OBJECT_INIT(object) \
    {                                \
        RT_NULL                      \
    }

/**
 * @brief initialize a single list
 * 初始化一个单向链表
 *
 * @param l the single list to be initialized
 */
rt_inline void rt_slist_init(rt_slist_t *l)
{
    l->next = RT_NULL;
}

/**
 * @brief 在单向链表 l 的尾部追加一个节点 n
 */
rt_inline void rt_slist_append(rt_slist_t *l, rt_slist_t *n)
{
    struct rt_slist_node *node;

    node = l;
    while (node->next)
    {
        node = node->next;
    }

    /* append the node to the tail */
    node->next = n;
    n->next    = RT_NULL;
}

/**
 * @brief 向单向链表节点 l 后插入一个节点 n
 */
rt_inline void rt_slist_insert(rt_slist_t *l, rt_slist_t *n)
{
    n->next = l->next;
    l->next = n;
}

/**
 * @brief 获取单向链表的长度
 */
rt_inline unsigned int rt_slist_len(const rt_slist_t *l)
{
    unsigned int      len  = 0;
    const rt_slist_t *list = l->next;
    while (list != RT_NULL)
    {
        list = list->next;
        len++;
    }

    return len;
}

/**
 * @brief 从单向链表 l 中移除节点 n
 */
rt_inline rt_slist_t *rt_slist_remove(rt_slist_t *l, rt_slist_t *n)
{
    /* remove slist head */
    struct rt_slist_node *node = l;
    while (node->next && node->next != n)
    {
        node = node->next;
    }

    /* remove node */
    if (node->next != (rt_slist_t *)0)
    {
        node->next = node->next->next;
    }

    return l;
}

/**
 * @brief 获取单向链表 l 的第一个节点
 */
rt_inline rt_slist_t *rt_slist_first(rt_slist_t *l)
{
    return l->next;
}

/**
 * @brief 获取单向链表 l 的最后一个节点
 */
rt_inline rt_slist_t *rt_slist_tail(rt_slist_t *l)
{
    while (l->next)
    {
        l = l->next;
    }

    return l;
}

/**
 * @brief 获取单向链表节点 n 的下一个节点
 */
rt_inline rt_slist_t *rt_slist_next(rt_slist_t *n)
{
    return n->next;
}

/**
 * @brief 判断单向链表 l 是否为空
 */
rt_inline int rt_slist_isempty(rt_slist_t *l)
{
    return l->next == RT_NULL;
}

/**
 * @brief get the struct for this single list node
 * 获取单向链表节点的数据结构
 *
 * @param node the entry point 入口点
 * @param type the type of structure 结构体类型
 * @param member the name of list in structure 结构体中链表的成员名
 */
#define rt_slist_entry(node, type, member) rt_container_of(node, type, member)

/**
 * @brief iterate over a single list
 * 遍历单向链表
 *
 * @pos:  the rt_slist_t * to use as a loop cursor. 指向宿主结构的指针, 在 for 循环中是一个迭代变量
 * @head: the head for your single list. 链表头
 */
#define rt_slist_for_each(pos, head) for (pos = (head)->next; pos != RT_NULL; pos = pos->next)

/**
 * @brief iterate over single list of given type
 * 循环遍历单向链表 head 中每一个 pos 中的 member 成员
 *
 * @pos:    the type * to use as a loop cursor. 指向宿主结构的指针, 在 for 循环中是一个迭代变量
 * @head:   the head for your single list. 单链表的链表头
 * @member: the name of the list_struct within the struct. 结构体中链表的成员名
 */
#define rt_slist_for_each_entry(pos, head, member)                                            \
    for (pos = rt_slist_entry((head)->next, typeof(*pos), member); &pos->member != (RT_NULL); \
         pos = rt_slist_entry(pos->member.next, typeof(*pos), member))

#define rt_slist_type_for_each_entry(type, pos, head, member)                                            \
    for (pos = rt_slist_entry((head)->next, type, member); &pos->member != (RT_NULL); \
         pos = rt_slist_entry(pos->member.next, type, member))

/**
 * @brief get the first element from a slist
 * 获取链表中的第一个元素
 *
 * @ptr:    the slist head to take the element from. 链表头
 * @type:   the type of the struct this is embedded in. 结构体类型
 * @member: the name of the slist_struct within the struct. 结构体中链表的成员名
 *
 * Note, that slist is expected to be not empty. 该链表不能为空
 */
#define rt_slist_first_entry(ptr, type, member) rt_slist_entry((ptr)->next, type, member)

/**
 * @brief get the tail element from a slist
 * 获取链表中的最后一个元素
 *
 * @ptr:    the slist head to take the element from. 链表头
 * @type:   the type of the struct this is embedded in. 结构体类型
 * @member: the name of the slist_struct within the struct.结构体中链表的成员名
 *
 * Note, that slist is expected to be not empty. 该链表不能为空
 */
#define rt_slist_tail_entry(ptr, type, member) rt_slist_entry(rt_slist_tail(ptr), type, member)

#ifdef __cplusplus
}
#endif