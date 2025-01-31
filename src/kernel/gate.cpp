#include <gate.h>
#include <console.h>
#include <stdio.h>
#include <interrupts.h>
#include <assert.h>
#include <syscall.h>
#include <task_queue.h>
#include <memory.h>

handler_t syscall_table[SYSCALL_SIZE];
static void sys_default()
{
    panic("syscall not implemented");
}

Task *task = NULL;
static uint32_t sys_test()
{
    
    BMB;
    memory::link_page(0x1600000);
    BMB;
    char* ptr = (char*)0x1600000;
    ptr[3] = 'T';

    BMB;
    memory::unlink_page(0x1600000);

    BMB;
    ptr[3] = 'T';

    return 255;
}

int32_t sys_write(fd_t fd, char* buf, uint32_t len)
{
    if(fd == stdout || fd == stderr)
    {
        console_write(buf, len);
        return 0;
    }
    panic("write!!!");
    return 0;
}

void syscall_init()
{
    for (size_t i = 0; i < SYSCALL_SIZE; i++)
    {
        syscall_table[i] = (handler_t)sys_default;
    }
    syscall_table[SYS_NR_TEST] = (handler_t)sys_test;
    syscall_table[SYS_NR_WRITE] = (handler_t)sys_write;
    syscall_table[SYS_NR_YIELD] = (handler_t)TaskQueue::task_yield; // 系统调用yield
    syscall_table[SYS_NR_SLEEP] = (handler_t)TaskQueue::task_sleep;
}

void syscall_check(uint32_t nr)
{
    if (nr >= SYSCALL_SIZE)
    {
        panic("syscall nr error");
    }
}
