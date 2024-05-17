#include "mutex.h"

#include <interrupts.h>
#include <task_queue.h>

/**
 * 初始化互斥量
*/
Mutex::Mutex()
{
    value = 0;
    wait_queue.init();
}

/**
 * 尝试持有互斥量
*/
void Mutex::lock()
{
    // 关闭中断，保证原子操作
    bool intr = InterruptManager::disable_interrupt();
    Task* current = TaskQueue::running_task();
    if(value >= 1) {
        // 若value为true，表示已经被别人持有
        // 则当前任务加入互斥量等待队列
        assert(current->node.next == NULL);
        assert(current->node.prev == NULL);
        wait_queue.push_back(&current->node);
        current->state = TASK_BLOCKED;
    }
    // 无人持有
    assert(value == 0);

    // 持有
    value++;
    assert(value == true);

    // 恢复中断状态
    InterruptManager::set_interrupt_state(intr);
}

/**
 * 释放互斥量
*/
void Mutex::unlock()
{
    // 关闭中断，保证原子操作
    bool intr = InterruptManager::disable_interrupt();

    Task* current = TaskQueue::running_task();

    // 互斥量已被持有
    assert(value);

    // 取消持有
    value--;
    assert(!value);

    //如果等待队列不为空，恢复第一个任务的执行
    if(!wait_queue.empty())
    {
        Task* task = element_entry(Task, node, wait_queue.get_head_node()->next);
        assert(task->magic == ONIX_MAGIC);
        TaskQueue::task_unblock(task);
        // 保证新进程能获得互斥量，不然可能饿死
        TaskQueue::task_yield();
    }

    // 恢复中断状态
    InterruptManager::set_interrupt_state(intr);
}
