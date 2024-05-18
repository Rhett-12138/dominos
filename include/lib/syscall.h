#pragma once

#include <types.h>
#define SYSCALL_SIZE 256

enum syscall_t {
    SYS_NR_TEST,
    SYS_NR_WRITE = 4,
    SYS_NR_BRK = 45,
    SYS_NR_YIELD = 158,
    SYS_NR_SLEEP = 162
};

uint32_t test();
void yield();
void sleep(uint32_t ms);

int32_t write(fd_t fd, char* buf, uint32_t len);