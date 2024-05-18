#include <stdio.h>
#include <stdarg.h>
#include <charBuf.h>
#include <console.h>
#include <syscall.h>

enum State
{
    INIT,
    GOTP,
    GOTSIZE1,
    GOTSIZE2,
    GOTTYPE
};
struct Format
{
    int flags;
    int length;
    int precision;
    Format(int len = 0, int pre = 6)
    {
        length = len;
        precision = pre;
        flags = 0;
    };
    void reset()
    {
        length = 0;
        precision = 6;
        flags = 0;
    };
};

int getValue(char ch)
{
    return ch - '0';
}

char getChar(int value)
{
    return (char)(value + '0');
}

int printk(const char *fmt, ...)
{
    va_list args;
    CharBuf buf;
    va_start(args, fmt);
    int res = vsprintf(buf, fmt, args);
    va_end(args);

    // 输出
    console_write(buf.getBuf(), buf.getLength());

    return res;
}

int printf(const char *fmt, ...)
{
    va_list args;
    CharBuf buf;
    va_start(args, fmt);
    int res = vsprintf(buf, fmt, args);
    va_end(args);

    // 输出 使用系统调用
    write(stdout, buf.getBuf(), buf.getLength());

    return res;
}

int vsprintf(CharBuf &buf, const char *fmt, va_list args)
{
    State state = State::INIT;
    Format format;
    char *s;
    unsigned long num;
    for (; *fmt; ++fmt)
    {
        if (*fmt != '%')
        {
            if (state == INIT)
            {
                buf.putChar(*fmt);
                continue;
            }
            else
            {
                switch (*fmt)
                {
                case '+':
                    format.flags |= PLUS;
                    break;
                case '-':
                    format.flags |= LEFT;
                    break;
                case '.':
                    state = GOTSIZE1;
                    if (!is_digit(*(fmt + 1)))
                    {
                        state = GOTSIZE2;
                    }
                    else
                    {
                        format.precision = 0;
                    }
                    break;
                case 'i':
                case 'd': // 带符号输出整数
                    format.flags |= SIGN;
                case 'u':
                    num = va_arg(args, unsigned long);
                    number(buf, num, 10, format.length, format.flags);
                    format.reset();
                    state = INIT;
                    break;
                case 'x': // 以16进制输出整数
                    format.flags |= SMALL;
                case 'X':
                    num = va_arg(args, unsigned long);
                    number(buf, num, 16, format.length, format.flags);
                    format.reset();
                    state = INIT;
                    break;
                case 'p':
                    format.length = 8;
                    format.flags |= ZEROPAD;
                    num = va_arg(args, unsigned long);
                    number(buf, num, 16, format.length, format.flags);
                    format.reset();
                    state = INIT;
                    break;
                case 's': // 输出字符串
                    s = va_arg(args, char *);
                    buf.putStr(s);
                    format.reset();
                    state = INIT;
                    break;
                case 'c': // 输出一个字符
                    buf.putChar((unsigned char)va_arg(args, int));
                    format.reset();
                    state = INIT;
                    break;
                case 'l':
                case 'f': // 输出浮点数
                    break;

                default:
                    if (is_digit(*fmt))
                    {
                        if (state == GOTP)
                        {
                            if (*fmt == '0' && format.length == 0)
                            {
                                format.flags |= ZEROPAD; // 填充0
                            }
                            format.length = format.length * 10 + getValue(*fmt);
                        }
                        else if (state == GOTSIZE1)
                        {
                            format.precision = format.precision * 10 + getValue(*fmt);
                        }
                    }
                    else
                    {
                        return -1; // 控制字符错误
                    }
                    break;
                }
            }
        }
        else
        {
            if (state == INIT)
            {
                state = GOTP;
                continue;
            }
            else
            {
                buf.putChar(*fmt);
                state = INIT;
                continue;
            }
        }
    }
}

int sprintf(char *buf, const char *fmt, ...)
{
    return 0;
}

/**
 * @brief 将整数转换为指定进制的字符串
 * @param buf 输出字符串缓冲区
 * @param num 整数
 * @param base 进制基数
 * @param size 字符串长度
 * @param flags 选项
 */
void number(CharBuf &buf, unsigned long num, int base, int size, int flags)
{
    char pad, sign, tmp[36];
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int index = 0;

    // 如果 flags 指出用小写字母，则定义小写字母集
    if (flags & SMALL)
        digits = "0123456789abcdefghijklmnopqrstuvwxyz";

    // 如果 flags 指出要左对齐，则屏蔽类型中的填零标志
    if (flags & LEFT)
        flags &= ~ZEROPAD;

    // 如果 flags 指出要填零，则置字符变量 c='0'，否则 c 等于空格字符
    pad = (flags & ZEROPAD) ? '0' : ' ';

    // 如果 flags 指出是带符号数并且数值 num 小于 0，则置符号变量 sign=负号，并使 num 取绝对值
    if (flags & SIGN && ((long)num) < 0)
    {
        // console_write("double");
        sign = '-';
        num = -((long)num);
    }
    else if (flags & SIGN && !(flags & DOUBLE) && ((int)num) < 0)
    {
        sign = '-';
        num = -((int)num);
    }
    else
    { // 否则如果 flags 指出是加号，则置 sign=加号，否则若类型带空格标志则 sign=空格，否则置 0
        sign = (flags & SIGN) ? '+' : ' ';
    }

    if (num == 0)
    {
        tmp[index++] = '0';
    }
    else
    {
        while (num)
        {
            tmp[index++] = digits[num % base];
            num /= base;
        }
    }

    if (flags & LEFT)
    {
        // 若带符号，则宽度值减 1
        if (flags & PLUS)
        {
            tmp[index++] = sign;
        }
        for (int i = index - 1; i >= 0; i--)
        {
            buf.putChar(tmp[i]);
        }
        size -= index; // 减去数本身的长度
        while (size > 0)
        {
            buf.putChar(' '); // 填充空格直到长度到达要求
            size--;
        }
    }
    else
    {
        if ((flags & PLUS) && pad == '0')
        {
            buf.putChar(sign);
            size--;
            size -= index;
            while (size > 0)
            {
                buf.putChar('0'); // 填充空格直到长度到达要求
                size--;
            }
        }
        else if (flags & PLUS)
        {
            size--;
            size -= index;
            while (size > 0)
            {
                buf.putChar(' '); // 填充空格直到长度到达要求
                size--;
            }
            buf.putChar(sign);
        }
        else
        {
            size -= index;
            while (size > 0)
            {
                buf.putChar(pad); // 填充空格直到长度到达要求
                size--;
            }
        }
        for (int i = index - 1; i >= 0; i--)
        {
            buf.putChar(tmp[i]);
        }
    }
}

void debug(char* file, int line, const char *fmt, ...)
{
    CharBuf buf;
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    printk("[%s] [%d] %s\n", file, line, buf.getBuf());
    // printk("%s", buf.getBuf());
    va_end(args);
}