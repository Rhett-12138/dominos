#pragma once

#include <types.h>
#include <stdio.h>

class FifoQueue
{
private:
    char* buf;
    uint32_t length;
    uint32_t head;
    uint32_t tail;
public:
    void init(char* t_buf, uint32_t length);
    uint32_t head_next();
    uint32_t tail_next();
    bool full();
    bool empty();

    char get_char();
    void put_char(char byte);
};

