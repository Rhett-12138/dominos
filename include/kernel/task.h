#pragma once

#include <types.h>

#define PAGE_SIZE 0x1000

typedef void target_t();

struct task_frame_t
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebi;
    uint32_t ebp;
    void (*eip)(void); // 返回值为void且不带参数的函数指针，调用: *(void(*)(void))()
};

class Task
{
private:
    uint32_t* stack; // 栈顶的位置
    task_frame_t frame;
public:
    Task(target_t target);
};