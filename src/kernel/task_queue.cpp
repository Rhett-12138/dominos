#include "task_queue.h"

#include <stdio.h>
#include <assert.h>
#include <interrupts.h>
#include <syscall.h>
#include <clock.h>

Task *TaskQueue::task_table[MAX_TASKS];
List TaskQueue::block_list = List();
List TaskQueue::sleep_list = List();
Task *idle_task;

/**
 * @brief 初始化任务队列
 */
void TaskQueue::init_queue()
{
    block_list.init();
    sleep_list.init();

    Task *task = running_task();
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
    if (task == nullptr && state == TASK_READY)
    {
        task = idle_task;
    }
    return task;
}

Task *TaskQueue::running_task()
{
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n");
}

void TaskQueue::schedule()
{
    assert(!InterruptManager::get_interrupt_state()); // 不可中断

    Task *current = running_task();
    Task *next = task_search(task_state_t::TASK_READY);
    if (next == nullptr)
    {
        return;
    }
    assert(next->magic == ONIX_MAGIC); // 检测栈溢出

    if (current->state == task_state_t::TASK_RUNNING)
    {
        current->state = task_state_t::TASK_READY;
    }

    next->state = task_state_t::TASK_RUNNING;

    if (next == current)
    {
        return;
    }
    task_switch(next);
}

Task *TaskQueue::task_create(target_t target, const char *name, uint32_t priority, uint32_t uid)
{
    Task *task = get_free_task();
    if (task == nullptr)
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

void TaskQueue::task_block(Task *task, List* blist, task_state_t state)
{
    assert(!InterruptManager::get_interrupt_state());
    assert(task->node.next == NULL);
    assert(task->node.prev == NULL);
    // LOG("block thread 0x%p", task);
    if (blist==nullptr)
    {
        blist = &block_list;
    }

    blist->push_back(&task->node);
    assert(state != TASK_READY && state != TASK_RUNNING);
    task->state = state;
    Task *current = running_task();
    if (current == task)
    {
        schedule();
    }
}

void TaskQueue::task_unblock(Task *task)
{
    assert(!InterruptManager::get_interrupt_state());
    // LOG("unblock thread 0x%p", task);
    block_list.remove(&task->node);
    // sleep_list.remove(&task->node); // remove函数只看node，不针对特定的node
    // 确认移除成功
    assert(task->node.next == NULL);
    assert(task->node.prev == NULL);

    task->state = TASK_READY;
}

void TaskQueue::task_sleep(uint32_t ms)
{
    assert(!InterruptManager::get_interrupt_state()); // not allow interrupt

    uint32_t ticks = ms / JIFFY;   // 获取睡眠的时间片
    ticks = ticks > 0 ? ticks : 1; // 至少休眠一个时间片

    Task *current = running_task();
    current->jiffies = Clock::get_jiffies() + ticks; // 唤醒时的全局时间片

    list_node_t *anchor = sleep_list.get_tail_node();

    for (list_node_t *ptr = sleep_list.get_head_node()->next; ptr != sleep_list.get_tail_node(); ptr = ptr->next)
    {
        Task *task = element_entry(Task, node, ptr);

        if (task->jiffies > current->jiffies)
        {
            anchor = ptr;
            break;
        }
    }

    // 插入睡眠链表
    task_block(current, &sleep_list, TASK_SLEEPING);

    schedule();
}

/**
 * 唤醒睡眠结束的任务
 */
void TaskQueue::task_wakeup()
{
    assert(!InterruptManager::get_interrupt_state()); // not allow interrupt
    if (sleep_list.empty())
    {
        return;
    }

    // 从睡眠链表中找到需要唤醒的任务
    for (list_node_t *ptr = sleep_list.get_head_node()->next; ptr != sleep_list.get_tail_node();)
    {
        Task *task = element_entry(Task, node, ptr);
        if (task->jiffies > Clock::get_jiffies())
        {
            break;
        }
        ptr = ptr->next;
        //
        task_unblock(task);
    }
}

extern void idle_thread();
extern void init_thread();
extern void test_thread();

void thread_a()
{
    InterruptManager::set_interrupt_state(true);
    while (true)
    {
        printf("thread A running\n");
        test();
    }
}
void thread_b()
{
    InterruptManager::set_interrupt_state(true);
    while (true)
    {
        printf("thread B running\n");
        test();
    }
}

void task_init()
{
    TaskQueue::init_queue();
    idle_task = TaskQueue::task_create((target_t *)idle_thread, "idle", 1, KERNEL_USER);
    TaskQueue::task_create((target_t *)init_thread, "init", 5, NORMAL_USER);
    TaskQueue::task_create((target_t *)test_thread, "test", 5, KERNEL_USER);
}
