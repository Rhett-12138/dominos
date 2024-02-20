#include <gate.h>
#include <stdio.h>
#include <interrupts.h>
#include <assert.h>
#include <syscall.h>

handler_t syscall_table[SYSCALL_SIZE];
static void sys_default()
{
    panic("syscall not implemented");
}

static uint32_t sys_test()
{
    LOG("syscall test ...");
    return 255;
}

void syscall_init()
{
    for(size_t i=0; i<SYSCALL_SIZE; i++)
    {
        syscall_table[i] = (handler_t)sys_default;
    }
    syscall_table[0] = (handler_t)sys_test;
}

void syscall_check(uint32_t nr)
{
    if(nr>=SYSCALL_SIZE)
    {
        panic("syscall nr error");
    }
}

