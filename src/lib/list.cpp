#include <list.h>

List::List()
{
    head.next = &tail;
    head.prev = nullptr;
    tail.prev = &head;
    tail.next = nullptr;
    max_size = MAX_LIST_SIZE;
}

List::List(int m_size)
{
    head.next = &tail;
    head.prev = nullptr;
    tail.prev = &head;
    tail.next = nullptr;
    max_size = m_size;
}

void List::init()
{
    head.next = &tail;
    head.prev = nullptr;
    tail.prev = &head;
    tail.next = nullptr;
}

/**
 * 在 anchor 结点前插入结点 node
 */
void List::insert_before(list_node_t *anchor, list_node_t *node)
{
    node->prev = anchor->prev;
    node->next = anchor;

    anchor->prev->next = node;
    anchor->prev = node;
}

/**
 * 在 anchor 结点后插入结点 node
 */
void List::insert_after(list_node_t *anchor, list_node_t *node)
{
    node->prev = anchor;
    node->next = anchor->next;

    anchor->next->prev = node;
    anchor->next = node;
}

/**
 * 在头结点后插入
 */
void List::push_front(list_node_t *node)
{
    assert(!search(node)); // node应不存在于list中
    insert_after(&head, node);
}

/**
 * 移除头节点后的结点并返回
 */
list_node_t *List::pop_front()
{
    if (empty())
    {
        return nullptr;
    }
    list_node_t *node = head.next;
    remove(node);
    return node;
}

/**
 * 在尾结点前插入
 */
void List::push_back(list_node_t *node)
{
    assert(!search(node));
    insert_before(&tail, node);
}

/**
 * 移除尾节点前的结点并返回
 */
list_node_t *List::pop_back()
{
    if (empty())
    {
        return nullptr;
    }
    list_node_t *node = tail.prev;
    remove(node);
    return node;
}

/**
 * 查找结点是否在链表中
 */
bool List::search(list_node_t *node)
{
    list_node_t *next = head.next;
    while (next != nullptr && next != &tail)
    {
        if (next == node)
        {
            return true;
        }
        next = next->next;
    }
    return false;
}

/**
 * 从链表中删除结点
 */
void List::remove(list_node_t *node)
{
    assert(node->prev != nullptr);
    assert(node->next != nullptr);

    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = nullptr;
    node->prev = nullptr;
}

/**
 * 链表是否为空
 */
bool List::empty()
{
    return (head.next == &tail);
}

/**
 * 获取链表长度
 */
uint32_t List::size() const
{
    uint32_t size = 0;
    list_node_t *next = head.next;
    while (next != &tail)
    {
        size++;
        next = next->next;
    }
    return size;
}

list_node_t *List::get_head_node()
{
    return &head;
}

list_node_t *List::get_tail_node()
{
    return &tail;
}

void list_test()
{
    uint32_t count = 5;
    List list;
    LOG("list address: 0x%p", &list);
    list_node_t *node;
    while (count--)
    {
        node = (list_node_t *)memory::alloc_kpage(1);
        list.push_front(node);
    }
    LOG("list size:%d\n", list.size());
    while (!list.empty())
    {
        node = list.pop_front();
        memory::free_page((uint32_t)node);
    }
    LOG("list size:%d\n", list.size());
}
