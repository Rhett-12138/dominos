#include <bitmap.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

Bitmap::Bitmap(/* args */)
{
}

Bitmap::Bitmap(char *bits, uint32_t length, uint32_t offset)
{
    make(bits, length, offset);
}

Bitmap::~Bitmap()
{
}

void Bitmap::init(char *bits, uint32_t length, uint32_t offset)
{
    memset(bits, 0, length);
    make(bits, length, offset);
}

void Bitmap::make(char *bits, uint32_t length, uint32_t offset)
{
    this->bits = (uint8_t *)bits;
    this->length = length;
    this->offset = offset;
}

/**
 * @brief // 测试位图的某一位是否为1
 * @param index 物理位置
 * @return - true 该位为1
 * @return - false 该为不为1
 */
bool Bitmap::test(uint32_t index)
{
    assert(index >= this->offset);
    idx_t idx = index - this->offset; // 得到位图的索引

    uint32_t bytes = idx / 8; // 位图数组中的哪个字节
    uint8_t bits = idx % 8;   // 该字节中的哪一位

    assert(bytes < this->length);

    return (this->bits[bytes] & (1 << bits));
}

/**
 * @brief 设置位图的某位的值
 */
void Bitmap::set(uint32_t index, bool value)
{
    assert(index >= this->offset);

    idx_t idx = index - this->offset; // 得到位图的索引

    uint32_t bytes = idx / 8; // 位图数组中的哪个字节
    uint8_t bits = idx % 8;   // 该字节中的哪一位
    if (value)
    {
        // 置为 1
        this->bits[bytes] |= (1 << bits);
    }
    else
    {
        // 置为 0
        this->bits[bytes] &= ~(1 << bits);
    }
}

/**
 * @brief 设置位图的从 index_start 位开始长度位 length 位的值
 */
void Bitmap::set(uint32_t index_start, uint32_t length, bool value)
{
    assert(length > 0);
    uint32_t index_end = index_start + length;
    assert(index_start >= this->offset);
    assert(index_end >= this->offset);

    idx_t idx = index_start - this->offset; // 得到位图的索引
    uint32_t bytes = idx / 8;               // 位图数组中的哪个字节
    uint8_t bits = idx % 8;                 // 该字节中的哪一位

    idx_t idx_end = index_end - this->offset; // 得到位图的索引
    uint32_t bytes_end = idx_end / 8;         // 位图数组中的哪个字节
    uint8_t bits_end = idx_end % 8;           // 该字节中的哪一位
    while (bytes <= bytes_end)
    {
        if (value)
        {
            // 置为 1
            this->bits[bytes] |= (1 << bits);
        }
        else
        {
            // 置为 0
            this->bits[bytes] &= ~(1 << bits);
        }
        bits++;
        if (bits == 8)
        {
            bytes++;
            bits = 0;
        }
        if (bytes == bytes_end && bits >= bits_end)
        {
            break;
        }
    }
}

/**
 * @brief 从位图中得到连续的count位
 * @param count count >= 1
 * @return - EOF 没有连续的 count 位
 * @return - index 找到的索引idx
 */
int Bitmap::scan(uint32_t count)
{
    assert(count >= 1);
    int start = EOF;                       // 标记目标开始的位置
    uint32_t bits_left = this->length * 8; // 剩余的位数
    uint32_t next_bit = 0;                 // 下一个位
    uint32_t counter = 0;                  // 计数器
    // 从头开始找
    while (bits_left-- > 0)
    {
        if (!test(offset + next_bit))
        {
            // 如果下一个位没有占用，则计数器加一
            counter++;
        }
        else
        {
            // 否则计数器置为 0，继续寻找
            counter = 0;
        }
        // 下一位，位置加一
        next_bit++;
        if (counter == count)
        {
            start = next_bit - count;
            break;
        }
    }
    // 如果没找到，则返回 EOF(END OF FILE)
    if (start == EOF)
        return EOF;

    // 否则将找到的位，全部置为 1
    set(offset + start, count, true);
    // bits_left = count;
    // next_bit = start;
    // while (bits_left--)
    // {
    //     set(offset + next_bit, true);
    //     next_bit++;
    // }
    // 然后返回索引
    return start + offset;
}

// char map[2];

// void Bitmap::tests()
// {
//     this->init(map, 2, 0);
//     for (size_t i = 1; i <= 33; i++)
//     {
//         uint32_t idx = scan(1);
//         if (idx == EOF)
//         {
//             LOG("Bitmap test finish.");
//             break;
//         }
//         LOG("i:%d, idx:%d", i, idx);
//         LOG("%x%x", map[0], map[1]);
//     }
// }
