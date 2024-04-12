#include <syscall.h>
#include <stdio.h>

/**
 * 0个参数的系统调用
*/
static inline uint32_t _syscall0(uint32_t nr)
{
    uint32_t ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(nr)
    );
    return ret;
}

uint32_t test()
{
    _syscall0(SYS_NR_TEST);
}

void yield()
{
    _syscall0(SYS_NR_YIELD);
}