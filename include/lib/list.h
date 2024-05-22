#pragma once

#include <types.h>
#include <stdio.h>
#include <memory.h>

#define element_offset(type, member) (uint32_t)(&((type *)0)->member)
#define element_entry(type, member, ptr) (type *)((uint32_t)ptr - element_offset(type, member))

#define MAX_LIST_SIZE 1024 * 1024

// 链表结点
struct list_node_t
{
    list_node_t *prev; // 前一结点
    list_node_t *next; // 下一结点
};

// 链表 - 带头节点和尾结点
class List
{
private:
    list_node_t head;
    list_node_t tail;
    int max_size;

public:
    List(/* args */);
    List(int m_size);
    void init();
    void insert_before(list_node_t *anchor, list_node_t *node); // 在 anchor 结点前插入结点 node
    void insert_after(list_node_t *anchor, list_node_t *node);  // 在 anchor 结点后插入结点 node
    void push_front(list_node_t *node);                         // 在头结点后插入
    list_node_t *pop_front();                                   // 移除头节点后的结点并返回
    void push_back(list_node_t *node);                          // 在尾结点前插入
    list_node_t *pop_back();                                    // 移除尾节点前的结点并返回
    bool search(list_node_t *node);                             // 查找结点是否在链表中
    static void remove(list_node_t *node);                             // 从链表中删除结点
    bool empty();                                               // 链表是否为空
    uint32_t size() const;                                      // 获取链表长度

    list_node_t* get_head_node();
    list_node_t* get_tail_node();
};

void list_test();
