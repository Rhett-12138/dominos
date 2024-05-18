#pragma once

#include <task.h>
#include <types.h>
#include <memory.h>
#include <list.h>
#define MAX_TASKS 64

class TaskQueue
{
private:
    static Task* task_table[MAX_TASKS];
    static List block_list;
    static List sleep_list;
public:
    static void init_queue();                     // 初始化任务队列
    static Task *get_free_task();                 // 为一个新任务获取内存空间
    static Task *task_search(task_state_t state); // 搜索处于某种状态的任务
    static Task *running_task();                  // 获取当前正在执行的任务
    static void schedule();                       // 任务切换
    static void task_switch(Task *next);
    static Task *task_create(target_t target, const char *name, uint32_t priority, uint32_t uid); // 创建任务
    static void task_yield();                                                                     // 进程主动切换

    static void task_block(Task *task, List* blist, task_state_t state);
    static void task_unblock(Task* task);

    static void task_sleep(uint32_t ms);
    static void task_wakeup(); // 唤醒睡眠结束的任务
};

// 测试
void task_init();