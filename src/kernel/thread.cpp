#include <interrupts.h>
#include <syscall.h>
#include <stdio.h>
#include <keyboard.h>
#include <task_queue.h>
#include <arena.h>

void idle_thread()
{
    InterruptManager::set_interrupt_state(false); // 开中断
    while (true)
    {
        // LOG("idle task...");
        asm volatile(
            "sti\n" // 开中断
            "hlt\n" // 关闭CPU，进入暂停状态，等待外中断的到来
        );
        yield(); // 放弃执行权，调度执行其他任务
    }
}

static void real_init_thread()
{
    uint32_t counter = 0;
    char ch;
    while (true)
    {
        printf("hello world\n");
        sleep(700);
    }
}

void init_thread()
{
    // char temp[100]; // 为了栈顶有足够的空间
    // LOG("to_user_mode, thread: 0x%p", TaskQueue::running_task());
    // TaskQueue::task_to_user_mode(real_init_thread);
    InterruptManager::set_interrupt_state(true);
    char ch;
    while (true)
    {
        // LOG("init_thread");
        bool intr = InterruptManager::disable_interrupt();
        Keyboard::read(&ch, 1);
        printk("%c", ch);
        InterruptManager::set_interrupt_state(intr);
    }
}

void test_thread()
{
    InterruptManager::set_interrupt_state(true);
    uint32_t counter = 0;
    while (true)
    {
        // LOG("test task %d...", counter++);
        void* ptr1 = kmalloc(1200);
        LOG("kmalloc 0x%p ...", ptr1);

        void* ptr2 = kmalloc(1024);
        LOG("kmalloc 0x%p ...", ptr2);

        void* ptr3 = kmalloc(54);
        LOG("kmalloc 0x%p ...", ptr3);
        kfree(ptr1);
        kfree(ptr2);
        kfree(ptr3);

        sleep(1709);
    }
}