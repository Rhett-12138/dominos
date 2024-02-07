#pragma once
#include <stdarg.h>
#include <string.h>
#include <charBuf.h>

#define ZEROPAD 1  // 填充0
#define SIGN 2     // usigned/signed long
#define DOUBLE 4   // 有效大小, 置位则为8字节
#define PLUS 8    // 显示符号
#define LEFT 16    // 左调整
#define SPECIAL 32 // 0x
#define SMALL 64   // 使用小写字母

#define is_digit(c) ((c) >= '0' && (c) <= '9')

// 对数字字符的处理
int getValue(char ch);
char getChar(int value);

void number(CharBuf& buf, unsigned long num, int base, int size, int flags);
int vsprintf(CharBuf& buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
int printf(const char *fmt, ...);
void debug(char* file, int line, const char *fmt, ...);

#define DEBUG(fmt, args...) debug(__BASE_FILE__, __LINE__, fmt, ##args)
#define LOG(fmt, args...) DEBUG(fmt, ##args)

#define BMB asm volatile("xchgw %bx, %bx")
// #define LOG(fmt, args...)