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
#include <bitmap.h>

char message[] = "hello world!"; // .data

extern "C" void kernel_init()
{
    // console_init();
    // gdt_init();

    memory::memory_map_init();
    memory::mapping_init();

    InterruptManager intManager;
    intManager.interrupt_init();

    // 时钟
    Clock clock(IRQ_CLOCK);

    // 系统时间
    Time::time_init();

    Bitmap bmap;
    
    // asm volatile("sti");
    int counter = 0;
    while (true)
    {
        // LOG("loop in kernel init%d\n", counter++);
        // delay(100000000);
    }

    return;
}