#pragma once

#include <types.h>
#include <list.h>
#include <task.h>

class Mutex
{
private:
    uint8_t value;      // 信号量
    List wait_queue; // 等待队列
public:
    Mutex();       // 初始化互斥量
    void lock();   // 尝试持有互斥量
    void unlock(); // 释放互斥量
};