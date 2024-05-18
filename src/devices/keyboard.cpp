#include "keyboard.h"

#include <stdio.h>
#include <task_queue.h>

bool Keyboard::capslock_state = false; // 大写锁定
bool Keyboard::scrlock_state = false;  // 滚动锁定
bool Keyboard::numlock_state = false;  // 数字锁定
bool Keyboard::extcode_state = false;  // 扩展码状态
FifoQueue Keyboard::fifo;
Lock Keyboard::lock = Lock();
Task *Keyboard::waiter;

void Keyboard::keyboard_init()
{
    InterruptManager::set_interrupt_handler(IRQ_KEYBOARD, (handler_t)&handler);
    fifo.init(buf, KEYBOARD_BUFFER_SIZE);
    lock.init();
    LOG("keyboard init... capslock %d", capslock_state);
    InterruptManager::set_interrupt_mask(IRQ_KEYBOARD, true);
}

void Keyboard::handler(int vector)
{
    assert(vector == 0x21);
    InterruptManager::send_eoi(vector);          // 发送中断处理完成信号
    uint16_t scancode = inb(KEYBOARD_DATA_PORT); // 从键盘读取按键信息扫描码
    // LOG("key raw code: 0x%x", scancode);
    uint8_t ext = 2; // keymap 状态索引，默认没有shift键

    // 扩展码字符
    if (scancode == 0xe0)
    {
        // 置扩展状态
        extcode_state = true;
        return;
    }

    // 扩展码
    if (extcode_state)
    {
        ext = 3;               // 该状态索引
        scancode |= 0xe000;    // 修改扫描码，添加0xe0前缀
        extcode_state = false; // 扩展状态无效
    }

    // 获得通码
    uint16_t makecode = (scancode & 0x7f);
    if (makecode == CODE_PRINT_SCREEN_DOWN)
    {
        makecode = KEY_PRINT_SCREEN;
    }
    // 生成码非法
    if (makecode > KEY_PRINT_SCREEN)
    {
        return;
    }

    // 是否是断码
    bool is_breakcode = ((scancode & 0x0080) != 0);
    if (is_breakcode)
    {
        keymap[makecode][ext] = false;
        return;
    }

    // 下面是通码，按键按下
    keymap[makecode][ext] = true;

    // 是否需要设置led灯
    bool set_led = false;
    if (makecode == KEY_NUMLOCK)
    {
        numlock_state = !numlock_state;
        set_led = true;
    }
    else if (makecode == KEY_CAPSLOCK)
    {
        capslock_state = !capslock_state;
        set_led = true;
    }
    else if (makecode == KEY_SCRLOCK)
    {
        scrlock_state = !scrlock_state;
        set_led = true;
    }
    if (set_led)
    {
        set_leds();
        return;
    }

    // 计算 shift 状态
    bool shift = false;
    if (capslock_state && ('a' <= keymap[makecode][0] && keymap[makecode][0] <= 'z')) // 大写锁定只对字母起作用
    {
        shift = !shift;
    }
    if (shift_state)
    {

        shift = !shift;
    }

    // 获得按键的 ASCII 码
    char ch = 0;
    // [/?] 这个键比较特殊，拓展码和普通码一样
    if (ext == 3 && makecode != KEY_SLASH)
    {
        ch = keymap[makecode][1];
    }
    else
    {
        ch = keymap[makecode][shift];
    }

    if (ch == INV)
        return;

    // 将字符存入缓冲区
    // LOG("key down %c", ch);
    fifo.put_char(ch);
    // keymap[makecode][ext] = false;
    if (waiter != nullptr)
    {
        TaskQueue::task_unblock(waiter);
        waiter = nullptr;
    }
}

void Keyboard::keyboard_wait()
{
    uint8_t state;
    do
    {
        state = inb(KEYBOARD_CTRL_PORT);
    } while (state & 0x02); // 读键盘缓冲区，直到为空
}

void Keyboard::keyboard_ack()
{
    uint8_t state;
    do
    {
        state = inb(KEYBOARD_DATA_PORT);
    } while (state != KEYBOARD_CMD_ACK);
}

void Keyboard::set_leds()
{
    uint8_t leds = (capslock_state << 2) | (numlock_state << 1) | scrlock_state;
    keyboard_wait();
    // 设置LED命令
    outb(KEYBOARD_DATA_PORT, KEYBOARD_CMD_LED);
    keyboard_ack();

    keyboard_wait();
    // 设置LED状态
    outb(KEYBOARD_DATA_PORT, leds);
    keyboard_ack();
}

uint32_t Keyboard::read(char *ptr, uint32_t count)
{
    lock.acquire();
    int nr = 0;
    while (nr < count)
    {
        while (fifo.empty())
        {
            waiter = TaskQueue::running_task();
            TaskQueue::task_block(waiter, nullptr, TASK_WAITING);
        }

        ptr[nr++] = fifo.get_char();
    }
    lock.release();
    return count;
}
