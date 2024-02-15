#pragma once

#include "types.h"

#define CRT_ADDR_REG 0x3D4 // CRT(6845)地址寄存器
#define CRT_DATA_REG 0x3D5 // CRT(6845)数据寄存器

#define CRT_START_ADDR_H 0xC // 显示内存起始位置 - 高位
#define CRT_START_ADDR_L 0xD // 显示内存起始位置 - 低位
#define CRT_CURSOR_H 0xE     // 光标位置 - 高位
#define CRT_CURSOR_L 0xF     // 光标位置 - 低位

#define MEM_BASE 0xB8000              // 显卡内存起始位置
#define MEM_SIZE 0x4000               // 显卡内存大小
#define MEM_END (MEM_BASE + MEM_SIZE) // 显卡内存结束位置
#define WIDTH 80                      // 屏幕文本列数
#define HEIGHT 25                     // 屏幕文本行数
#define ROW_SIZE (WIDTH * 2)          // 每行字节数
#define SCR_SIZE (ROW_SIZE * HEIGHT)  // 屏幕字节数

#define ASCII_NUL 0x00
#define ASCII_ENQ 0x05
#define ASCII_BEL 0x07  // \a
#define ASCII_BS 0x08   // \b  
#define ASCII_HT 0x09   // \t
#define ASCII_LF 0x0A   // \n
#define ASCII_VT 0x0B   // \v
#define ASCII_FF 0x0C   // \f
#define ASCII_CR 0x0D   // \r
#define ASCII_DEL 0x0F

extern "C" uint8_t inb(uint16_t port);  // input 1 byte
extern "C" void outb(uint16_t port, uint8_t value);  // output 1 byte

extern "C" uint16_t inw(uint16_t port); // input 1 word
extern "C" void outw(uint16_t port, uint16_t value); // output 1 word

extern "C" uint32_t inl(uint16_t port); // 输入一个双字
extern "C" void outl(uint16_t port, uint32_t value); // 输出一个双字