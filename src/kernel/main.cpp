#include <stdio.h>
#include <types.h>
#include <io.h>
#include <string.h>
#include <assert.h>
#include <interrupts.h>
#include <stdlib.h>
#include <clock.h>
#include <time.h>
#include <memory.h>
#include <task_queue.h>
#include <gate.h>
#include <list.h>

char message[] = "hello world!"; // .data

extern "C" void kernel_init()
{
    // console_init();
    // gdt_init();

    memory::memory_map_init();
    memory::mapping_init();

    InterruptManager::interrupt_init();

    // 时钟
    Clock clock(IRQ_CLOCK);

    // 系统时间
    Time::time_init();

    task_init();
    
    syscall_init();
    // memory::memory_test();
    
    // InterruptManager::set_interrupt_state(true); // 打开中断
    list_test();

}