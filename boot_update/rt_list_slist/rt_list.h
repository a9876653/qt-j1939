#pragma once

#include "rt_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Double List structure
 * 双向链表节点
 */
typedef struct rt_list_node
{
    struct rt_list_node *next; /**< point to next node. */
    struct rt_list_node *prev; /**< point to prev node. */
} rt_list_t;

/**
 * @brief initialize a list object
 * 初始化链表对象
 */
#define RT_LIST_OBJECT_INIT(object) \
    {                               \
        &(object), &(object)        \
    }

/**
 * @brief initialize a list
 * 初始化链表
 *
 * @param l list to be initialized. 将被初始化的链表
 */
rt_inline void rt_list_init(rt_list_t *l)
{
    l->next = l->prev = l;
}

/**
 * @brief insert a node after a list
 * 链表头插
 *
 * @param l list to insert it. 操作的链表
 * @param n new node to be inserted. 
 */
rt_inline void rt_list_insert_after(rt_list_t *l, rt_list_t *n)
{
    l->next->prev = n;
    n->next       = l->next;

    l->next = n;
    n->prev = l;
}

/**
 * @brief insert a node before a list
 * 链表尾插
 *
 * @param l list to insert it. 操作链表
 * @param n new node to be inserted. 将要被插入的新节点
 */
rt_inline void rt_list_insert_before(rt_list_t *l, rt_list_t *n)
{
    l->prev->next = n;
    n->prev       = l->prev;

    l->prev = n;
    n->next = l;
}

/**
 * @brief remove node from list.
 * 移除一个链表节点
 *
 * @param n the node to remove from the list. 将要从链表中删除的节点
 */
rt_inline void rt_list_remove(rt_list_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

/**
 * @brief tests whether a list is empty
 * 判断链表是否为空
 *
 * @param l the list to test. 被测试的链表
 */
rt_inline int rt_list_isempty(const rt_list_t *l)
{
    return l->next == l;
}

/**
 * @brief get the list length
 * 获取链表长度
 *
 * @param l the list to get. 被获取的链表
 */
rt_inline unsigned int rt_list_len(const rt_list_t *l)
{
    unsigned int     len = 0;
    const rt_list_t *p   = l;

    while (p->next != l)
    {
        p = p->next;
        len++;
    }

    return len;
}

/**
 * @brief get the struct for this entry
 * 获取结构体变量的地址
 *
 * @param node the entry point. 入口点
 * @param type the type of structure. 结构体的类型
 * @param member the name of list in structure. 结构体中链表的成员名
 */
#define rt_list_entry(node, type, member) rt_container_of(node, type, member)

/**
 * rt_list_for_each - iterate over a list
 * 遍历链表
 *
 * @pos:    the rt_list_t * to use as a loop cursor. 指向宿主结构的指针, 在 for 循环中是一个迭代变量
 * @head:   the head for your list. 链表头
 */
#define rt_list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * @brief iterate over a list safe against removal of list entry
 * 安全地遍历链表
 *
 * @pos:    the rt_list_t * to use as a loop cursor. 指向宿主结构的指针, 在 for 循环中是一个迭代变量
 * @n:      another rt_list_t * to use as temporary storage. 用于临时存储下一个数据结构的指针变量
 * @head:   the head for your list. 链表头
 */
#define rt_list_for_each_safe(pos, n, head) for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * @brief iterate over list of given type
 * 循环遍历 head 链表中每一个 pos 中的 member 成员
 *
 * @pos:    the type * to use as a loop cursor.  指向宿主结构的指针, 在for循环中是一个迭代变量
 * @head:   the head for your list. 链表头
 * @member: the name of the list_struct within the struct. 结构体中链表的成员名
 */
#define rt_list_for_each_entry(pos, head, member)                                         \
    for (pos = rt_list_entry((head)->next, typeof(*pos), member); &pos->member != (head); \
         pos = rt_list_entry(pos->member.next, typeof(*pos), member))

/**
 * @brief iterate over list of given type safe against removal of list entry.
 * @pos:    the type * to use as a loop cursor. 指向宿主结构的指针, 在for循环中是一个迭代变量
 * @n:      another type * to use as temporary storage. 用于临时存储下一个数据结构的指针变量
 * @head:   the head for your list. 链表头
 * @member: the name of the list_struct within the struct. 结构体中链表的成员名
 */
#define rt_list_for_each_entry_safe(pos, n, head, member)                                                                    \
    for (pos = rt_list_entry((head)->next, typeof(*pos), member), n = rt_list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                                                                                             \
         pos = n, n = rt_list_entry(n->member.next, typeof(*n), member))

/**
 * @brief get the first element from a list
 * 获取链表中的第一个元素
 *
 * @ptr:    the list head to take the element from. 链表头
 * @type:   the type of the struct this is embedded in. 结构体类型
 * @member: the name of the list_struct within the struct. 结构体中链表的成员名
 *
 * Note, that list is expected to be not empty. 该链表不能为空
 */
#define rt_list_first_entry(ptr, type, member) rt_list_entry((ptr)->next, type, member)

#ifdef __cplusplus
}
#endif