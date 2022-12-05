# 数据结构 - 单向链表与双向链表的代码实现

## container_of

```C
/**
 * @brief return the member address of ptr, if the type of ptr is the struct type.
 * 获取 type 结构体中, member 成员在这个结构体中的偏移, 从而得到 type 结构体的首地址, 后续可强转得到结构体的全部变量
 */
#define rt_container_of(ptr, type, member) ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

```



## 单向链表

```C
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
```



## 双向链表

```C
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
```