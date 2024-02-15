#pragma once
#include <io.h>
#include <types.h>

extern "C" void console_init();
void console_clear();
void console_write(char *buf, uint32_t count = 0);
void console_newline(int line = 1);
