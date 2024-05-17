#pragma once
#include <interrupts.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_CTRL_PORT 0x64

class Keyboard
{
private:
    Keyboard();
public:
    static void keyboard_init();
    static void handler(int vector);
};