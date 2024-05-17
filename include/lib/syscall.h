#pragma once

#include <types.h>
#define SYSCALL_SIZE 64

enum syscall_t {
    SYS_NR_TEST,
    SYS_NR_YIELD,
    SYS_NR_SLEEP
};

uint32_t test();
void yield();
void sleep(uint32_t ms);