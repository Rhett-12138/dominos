#pragma once

#include <types.h>
#include <bitmap.h>
#include <memory.h>

#define KERNEL_USER 0
#define NORMAL_USER 1
#define TASK_NAME_LEN 16

#define NR_TASKS 64

typedef void target_t();

enum task_state_t
{
    TASK_INIT,     // 初始化
    TASK_READY,    // 就绪
    TASK_RUNNING,  // 执行
    TASK_BLOCKED,  // 阻塞
    TASK_SLEEPING, // 睡眠
    TASK_WAITING,  // 等待
    TASK_DIED,     // 死亡
};

struct task_frame_t
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    void (*eip)(void); // 返回值为void且不带参数的函数指针，调用: *(void(*)(void))()
};

class Task
{
private:
    uint32_t *stack;              // 栈顶的位置
public:
    task_state_t state;           // 任务状态
    uint32_t priority;            // 任务优先级
    uint32_t ticks;               // 剩余时间片
    uint32_t jiffies;             // 上次执行时全局时间片
    char name[TASK_NAME_LEN]; // 任务名称
    uint32_t uid;                 // 用户id
    uint32_t pde;                 // 页目录物理地址
    Bitmap *vmap;                 // 进程虚拟内存位图
    uint32_t magic;               // 内核魔数，用于检测栈溢出
    task_frame_t frame;

public:
    Task();
    void create(target_t target, const char *name, uint32_t priority, uint32_t uid);
};