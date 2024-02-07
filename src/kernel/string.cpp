#include "string.h"
#include "types.h"

char *strcpy(char *dest, const char *src)
{
    char *ptr = dest;
    while (true)
    {
        *ptr++ = *src;
        if (*src == EOS)
        {
            return dest;
        }
        src++;
    }
}

char *strcat(char *dest, const char *src)
{
    char *ptr = dest;
    while (*ptr != EOS)
    {
        ptr++;
    }
    while (true)
    {
        *ptr = *src;
        if (*src == EOS)
        {
            return dest;
        }
        ptr++;
        src++;
    }
}

size_t strlen(const char *str)
{
    char *ptr = (char *)str;
    while (*ptr != EOS)
    {
        ptr++;
    }
    return ptr - str;
}

int strcmp(const char *lhs, const char *rhs)
{
    char *t_lhs = (char *)lhs;
    char *t_rhs = (char *)rhs;
    while ((*t_lhs == *t_rhs) && (*t_lhs != EOS && *t_rhs != EOS))
    {
        t_lhs++;
        t_rhs++;
    }
    return *t_lhs < *t_rhs ? -1 : *t_lhs > *t_rhs;
}

char *strchr(const char *str, int ch)
{
    char *ptr = (char *)str;
    while (true)
    {
        if (*ptr == ch)
        {
            return ptr;
        }
        if (*ptr++ == EOS)
        {
            return NULL;
        }
    }
}
char *strrchr(const char *str, int ch)
{
    char *last = NULL;
    char *ptr = (char *)str;
    while (true)
    {
        if (*ptr == ch)
        {
            return last = ptr;
        }
        if (*ptr++ == EOS)
        {
            return last;
        }
    }
}

int memcmp(const void *lhs, const void *rhs, size_t count)
{
    char *t_lhs = (char *)lhs;
    char *t_rhs = (char *)rhs;
    while (*t_lhs == *t_rhs && count > 0)
    {
        t_lhs++;
        t_rhs++;
        count--;
    }
}
void *memset(void *dest, int ch, size_t count)
{
    char *ptr = (char *)dest;
    while (count--)
    {
        *ptr++ = ch;
    }
    return dest;
}
void *memcpy(void *dest, const void *src, size_t count)
{
    char *ptr = (char *)dest;
    char *t_src = (char*)src;
    while (count--)
    {
        *ptr++ = *t_src++;
    }
    return dest;
}

void *memchr(const void *str, int ch, size_t count)
{
    char* ptr = (char*)str;
    while (count--)
    {
        if(*ptr==ch)
        {
            return (void*)ptr;
        }
        ptr++;
    }
}