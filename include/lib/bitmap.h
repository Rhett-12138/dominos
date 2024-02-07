#pragma once

#include <types.h>

class Bitmap
{
private:
    /* data */
    uint8_t *buffer; // 位图缓冲区
    uint32_t length; // 位图缓冲区长度
    uint32_t offset; // 位图开始的偏移
public:
    Bitmap(/* args */);
    Bitmap(char *buffer, uint32_t length, uint32_t offset); // 初始化位图
    ~Bitmap();
    void init(char *buffer, uint32_t length, uint32_t offset);      // 初始化位图
    void make(char *buffer, uint32_t length, uint32_t offset);      // 构造位图
    bool test(uint32_t index, uint32_t count = 1);                  // 测试位图中某一段的值是否为1
    void set(uint32_t index_start, bool value, uint32_t count = 1); // 设置位图的某一段的值
    int scan(uint32_t count);                                       // 从位图中得到连续的count位

    // void tests();
};
