#pragma once

#include <types.h>
#include <list.h>

#define DESC_COUNT 7

typedef list_node_t block_t; // 内存块

// 内存描述符
struct arena_descriptor_t
{
    uint32_t total_block; // 一页内存分成多少块
    uint32_t block_size;  // 块大小
    List free_list;       // 空闲列表
};

// 一页或多页内存
struct arena_t
{
    arena_descriptor_t *desc; // 该 arena 的描述符
    uint32_t count;           // 当前剩余多少块(没超过大小)，页数(超过大小)
    uint32_t large;           // 表示是不是超过了大小 (max: 1024 bytes)
    uint32_t magic;           // 魔数
};

void arena_init();
static void* get_arena_block(arena_t *arena, uint32_t idx);
static arena_t* get_block_arena(block_t* block);
void *kmalloc(size_t size);
void kfree(void *ptr);