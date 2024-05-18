#include "fifo.h"

void FifoQueue::init(char *t_buf, uint32_t t_length)
{
    buf = t_buf;
    length = t_length;
    head = 0;
    tail = 0; 
}

/**
 * 获取头指针下一位置
*/
uint32_t FifoQueue::head_next()
{
    return (head + 1) % length;
}

/**
 * 获取尾指针下一位置
*/
uint32_t FifoQueue::tail_next()
{
    return (tail + 1) % length;
}

bool FifoQueue::full()
{
    return tail_next() == head;
}

bool FifoQueue::empty()
{
    return head == tail;
}

char FifoQueue::get_char()
{
    assert(!empty());
    char byte = buf[head];
    head = head_next();
    return byte;
}

void FifoQueue::put_char(char byte)
{
    while(full()) {
        get_char();
    }
    buf[tail] = byte;
    tail = tail_next();
}
