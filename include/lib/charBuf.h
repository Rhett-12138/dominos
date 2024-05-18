#pragma once

#define BUFOVERFLOW -1
#define WRITEOK 0
#include <types.h>

class CharBuf
{
private:
    char buf[1024];
    uint32_t index;
public:
    CharBuf();
    int putStr(const char* str); // 写入字符串
    int putChar(const char ch); // 写入一个字符
    uint32_t getLength(); // 获取字符长度
    void clear();
    char* getBuf();
};