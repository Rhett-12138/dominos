#include "charBuf.h"
#include "types.h"

CharBuf::CharBuf()
{
    this->clear(); // 系统可能分配相同的空间从而感染脏数据
}

int CharBuf::putStr(const char *str)
{
    char *p = (char *)str;
    while (*p != EOS)
    {
        buf[index++] = *p;
        if (index == 1024)
        {
            return BUFOVERFLOW;
        }
        p++;
    }
    return WRITEOK;
}

int CharBuf::putChar(const char ch)
{
    buf[index++] = ch;
    if (index == 1024)
    {
        return BUFOVERFLOW;
    }
    return WRITEOK;
}

uint32_t CharBuf::getLength()
{
    return index;
}

void CharBuf::clear()
{
    for(int i=0; i<1024; i++)
    {
        buf[i] = '\0';
    }
    index = 0;
}

char *CharBuf::getBuf()
{
    return this->buf;
}
