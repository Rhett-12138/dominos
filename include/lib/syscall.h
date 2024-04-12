#pragma once

#include <types.h>
#define SYSCALL_SIZE 64

enum syscall_t {
    SYS_NR_TEST,
    SYS_NR_YIELD
};

uint32_t test();
void yield();