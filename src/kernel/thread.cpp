#include <interrupts.h>
#include <syscall.h>
#include <stdio.h>
#include <keyboard.h>

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

void init_thread()
{
    InterruptManager::set_interrupt_state(true);
    char ch;
    while (true)
    {
        // LOG("init_thread");
        bool intr = InterruptManager::disable_interrupt();
        Keyboard::read(&ch, 1);
        printf("%c", ch);
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
        sleep(1709);
    }
}