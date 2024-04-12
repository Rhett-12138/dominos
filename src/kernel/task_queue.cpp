#include "task_queue.h"

#include <stdio.h>
#include <assert.h>
#include <interrupts.h>
#include <syscall.h>

Task* TaskQueue::task_table[MAX_TASKS];

/**
 * @brief 初始化任务队列
 */
void TaskQueue::init_queue()
{
    Task* task = running_task();
    task->magic = ONIX_MAGIC;
    task->ticks = 1;
    for (size_t i = 0; i < MAX_TASKS; i++)
    {
        task_table[i] = nullptr;
    }
}

/**
 * @brief 为一个新任务获取空闲内存空间
 * @return - Task* 新任务的指针
 * @return - nullptr 任务队列已满
 */
Task *TaskQueue::get_free_task()
{
    for (size_t i = 0; i < MAX_TASKS; i++)
    {
        if (task_table[i] == nullptr)
        {
            task_table[i] = (Task *)memory::alloc_kpage(1);
            return task_table[i];
        }
    }
    return nullptr;
}

Task *TaskQueue::task_search(task_state_t state)
{
    assert(!InterruptManager::get_interrupt_state());
    Task *task = nullptr;
    Task *current = running_task();
    for (size_t i = 0; i < MAX_TASKS; i++)
    {
        Task *ptr = task_table[i];
        if (ptr == nullptr)
        {
            continue;
        }
        if (ptr->state != state)
        {
            continue;
        }
        if (current == ptr)
        {
            continue;
        }
        if (task == nullptr || task->ticks < ptr->ticks || ptr->jiffies < task->jiffies)
        {
            task = ptr;
        }
    }
    return task;
}

Task *TaskQueue::running_task()
{
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n"
    );
}

void TaskQueue::schedule()
{
    assert(!InterruptManager::get_interrupt_state()); // 不可中断

    Task *current = running_task();
    Task *next = task_search(task_state_t::TASK_READY);
    if(next==nullptr)
    {
        return;
    }
    assert(next->magic == ONIX_MAGIC); // 检测栈溢出

    if(current->state == task_state_t::TASK_RUNNING)
    {
        current->state = task_state_t::TASK_READY;
    }

    next->state = task_state_t::TASK_RUNNING;
    
    if(next==current)
    {
        return;
    }
    task_switch(next);
}

Task *TaskQueue::task_create(target_t target, const char *name, uint32_t priority, uint32_t uid)
{
    Task* task = get_free_task();
    if(task==nullptr)
    { // 任务队列已满
        return nullptr;
    }
    memset(task, 0, PAGE_SIZE);
    task->create(target, name, priority, uid);
}

void TaskQueue::task_yield()
{
    schedule();
}

uint32_t thread_a()
{
    InterruptManager::set_interrupt_state(true);
    while(true)
    {
        printf("task running...thread a.\n");
        // TaskQueue::task_yield();
        yield();
    }
}
uint32_t thread_b()
{
    InterruptManager::set_interrupt_state(true);
    while(true)
    {
        printf("task running...thread b.\n");
        // TaskQueue::task_yield();
        yield();
    }
}
uint32_t thread_c()
{
    InterruptManager::set_interrupt_state(true);
    while(true)
    {
        printf("task running...thread c.\n");
        // TaskQueue::task_yield();
        yield();
    }
}

void task_init()
{
    TaskQueue::init_queue();
    TaskQueue::task_create((target_t*)thread_a, "a", 5, KERNEL_USER);
    TaskQueue::task_create((target_t*)thread_b, "b", 5, KERNEL_USER);
    TaskQueue::task_create((target_t*)thread_c, "c", 5, KERNEL_USER);
}
