#pragma once

#include <task.h>
#include <types.h>
#include <memory.h>
#define MAX_TASKS 64

class TaskQueue
{
private:
    static Task *task_table[MAX_TASKS];

public:
    static void init_queue();                     // 初始化任务队列
    static Task *get_free_task();                 // 为一个新任务获取内存空间
    static Task *task_search(task_state_t state); // 搜索处于某种状态的任务
    static Task *running_task();                  // 获取当前正在执行的任务
    static void schedule();                       // 任务切换
    static void task_switch(Task *next);
    static Task *task_create(target_t target, const char *name, uint32_t priority, uint32_t uid); // 创建任务
};

// 测试
void task_init();