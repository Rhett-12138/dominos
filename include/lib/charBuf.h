#pragma once

#define BUFOVERFLOW -1
#define WRITEOK 0

class CharBuf
{
private:
    char buf[1024];
    int index;
public:
    CharBuf();
    int putStr(const char* str); // 写入字符串
    int putChar(const char ch); // 写入一个字符
    int getLength(); // 获取字符长度
    void clear();
    char* getBuf();
};