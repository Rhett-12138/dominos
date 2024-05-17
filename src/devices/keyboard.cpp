#include "keyboard.h"

#include <stdio.h>

void Keyboard::keyboard_init()
{
    InterruptManager::set_interrupt_handler(IRQ_KEYBOARD, (handler_t)&handler);
    LOG("keyboard init...");
    InterruptManager::set_interrupt_mask(IRQ_KEYBOARD, true);
}

void Keyboard::handler(int vector)
{
    assert(vector == 0x21);
    InterruptManager::send_eoi(vector); // 发送中断处理完成信号
    uint16_t scancode = inb(KEYBOARD_DATA_PORT); // 从键盘读取按键信息扫描码
    LOG("keyboard input 0x%X", scancode);
}


