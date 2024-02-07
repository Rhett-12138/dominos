#include <console.h>
#include <string.h>

static uint16_t *video = (uint16_t *)MEM_BASE;
static uint16_t cursor_x = 0; // 光标坐标
static uint16_t cursor_y = 0;

static uint32_t pos;    // 当前光标的内存位置
static uint32_t screen; // 当前显示器开始的内存位置

static uint8_t attr = 7;        // 字符样式
static uint16_t erase = 0x0720; // 空格

static void get_screen()
{
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    screen = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    screen |= inb(CRT_DATA_REG);

    screen <<= 1; // screen*=2
    screen += MEM_BASE;
}

static void set_screen()
{
    uint32_t temp = screen;
    temp -= MEM_BASE;
    temp >>= 1;
    outb(CRT_ADDR_REG, CRT_START_ADDR_H);
    outb(CRT_DATA_REG, (temp >> 8) & 0xff);
    outb(CRT_ADDR_REG, CRT_START_ADDR_L);
    outb(CRT_DATA_REG, temp & 0xff);
}

static void get_cursor()
{
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    pos = inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    pos |= inb(CRT_DATA_REG);

    pos <<= 1; // screen*=2
    pos += MEM_BASE;
}

static void set_cursor()
{
    uint32_t temp = pos;
    temp -= MEM_BASE;
    temp >>= 1;
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    outb(CRT_DATA_REG, (temp >> 8) & 0xff);
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    outb(CRT_DATA_REG, temp & 0xff);
}

static void reset_cursor()
{
    uint16_t cursor = 80 * cursor_y + cursor_x;
    outb(CRT_ADDR_REG, CRT_CURSOR_H);
    outb(CRT_DATA_REG, (cursor >> 8) & 0xff);
    outb(CRT_ADDR_REG, CRT_CURSOR_L);
    outb(CRT_DATA_REG, cursor & 0xff);
}

extern "C" void console_init()
{
    console_clear();
}

void console_clear()
{
    screen = MEM_BASE;
    pos = MEM_BASE;
    cursor_x = cursor_y = 0;
    set_cursor();
    set_screen();
    uint16_t *ptr = (uint16_t *)MEM_BASE;
    while (ptr < (uint16_t *)MEM_END)
    {
        *ptr++ = erase;
    }
}

/**
 * @brief delete one character before cursor
 */
static void command_bs()
{
    uint16_t cursor;
    if (cursor_x > 0)
    {
        cursor_x--;
        cursor = 80 * cursor_y + cursor_x;
        video[cursor] = (video[cursor] & 0xff00) | ' ';
    }
    else
    {
        if (cursor_y > 0) // the past line has content
        {
            cursor_y--;
            cursor_x = 79;
            cursor = 80 * cursor_y + cursor_x;
            if ((video[cursor] & 0xff) != ' ')
            { // delete this character and stay cursor here
                video[cursor] = (video[cursor] & 0xff00) | ' ';
            }
            else
            { // stay at the farest blankspace
                for (cursor_x = 78; cursor_x >= 0; cursor_x--)
                {
                    cursor = 80 * cursor_y + cursor_x;
                    if ((video[cursor] & 0xff) != ' ')
                    {
                        cursor_x++;
                        break;
                    }
                }
            }
        }
    }
    reset_cursor();
}

/**
 * @brief delete 1 character at cursor
 */
static void command_del()
{
    uint16_t cursor = 80 * cursor_y + cursor_x;
    video[cursor] = (video[cursor] & 0xff00) | ' ';
}

/**
 * @brief \\n
 */
static void command_lf()
{
    cursor_y++;
    if (cursor_y >= HEIGHT)
    {
        console_newline();
    }
}

static void command_cr()
{
    cursor_x = 0;
    reset_cursor();
}

static void start_beep()
{

}

void console_newline(int line)
{
    cursor_y -= line;
    uint16_t *base = (uint16_t *)MEM_BASE;
    uint16_t *new_base = (uint16_t *)(MEM_BASE + line * ROW_SIZE);
    while (new_base != (uint16_t *)MEM_END)
    {
        *base = *new_base;
        base++;
        new_base++;
    }
    while (base != (uint16_t *)MEM_END)
    {
        *base = erase;
        base++;
    }
}

void console_write(char *buf, uint32_t count)
{
    char ch;
    if (count == 0)
    {
        count = strlen(buf);
    }
    while (count > 0)
    {
        ch = *buf++;
        count--;
        switch (ch)
        {
        case ASCII_NUL:
            /* code */
            break;
        case ASCII_ENQ:
            break;
        case ASCII_BEL: // \a
            start_beep();
            break;
        case ASCII_BS: // \b
            command_bs();
            break;
        case ASCII_HT: // \t
            break;
        case ASCII_LF: // \n
            command_lf();
            command_cr();
            break;
        case ASCII_VT: // \v
            break;
        case ASCII_FF: // \f
            break;
        case ASCII_CR: // \r
            command_cr();
            break;
        case ASCII_DEL:
            command_del();
            break;
        default:
            uint16_t cursor = 80 * cursor_y + cursor_x;
            video[cursor] = (video[cursor] & 0xff00) | ch;
            cursor_x++;
            if (cursor_x >= 80)
            {
                command_lf();
                command_cr();
            }
            break;
        }
    }
    reset_cursor();
}
