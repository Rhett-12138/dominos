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

/**
 * 1个参数
*/
static inline uint32_t _syscall1(uint32_t nr, uint32_t arg)
{
    uint32_t ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(nr), "b"(arg)
    );
    return ret;
}

/**
 * 2个参数
*/
static inline uint32_t _syscall2(uint32_t nr, uint32_t arg1,  uint32_t arg2)
{
    uint32_t ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(nr), "b"(arg1), "c"(arg2)
    );
    return ret;
}

/**
 * 1个参数
*/
static inline uint32_t _syscall3(uint32_t nr, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    uint32_t ret;
    asm volatile(
        "int $0x80\n"
        : "=a"(ret)
        : "a"(nr), "b"(arg1), "c"(arg2), "d"(arg3)
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

void sleep(uint32_t ms)
{
    _syscall1(SYS_NR_SLEEP, ms);
}

int32_t write(fd_t fd, char *buf, uint32_t len)
{
    _syscall3(SYS_NR_WRITE, fd, (uint32_t)buf, len);
}
