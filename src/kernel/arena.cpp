#include "arena.h"

#include <memory.h>
#include <stdlib.h>

static arena_descriptor_t descriptors[DESC_COUNT];

// arena 初始化
void arena_init()
{
    uint32_t block_size = 16;
    for (size_t i = 0; i < DESC_COUNT; i++)
    {
        arena_descriptor_t *desc = &descriptors[i];
        desc->block_size = block_size;
        desc->total_block = (PAGE_SIZE - sizeof(arena_t)) / block_size;
        desc->free_list.init();
        block_size <<= 1; // block_size *= 2
    }
}

// 获得arena 第 idx 块内存指针
void *get_arena_block(arena_t *arena, uint32_t idx)
{
    assert(arena->desc->total_block > idx);
    void *addr = (void *)(arena + 1);
    uint32_t gap = idx * arena->desc->block_size;
    return addr + gap;
}

arena_t *get_block_arena(block_t *block)
{
    return (arena_t *)((uint32_t)block & 0xfffff000);
}

void *kmalloc(size_t size)
{
    arena_descriptor_t *desc = nullptr;
    arena_t *arena;
    block_t *block;
    char *addr;
    if (size > 1024)
    {
        uint32_t asize = size + sizeof(arena_t);
        uint32_t count = div_round_up(asize, PAGE_SIZE);
        arena = (arena_t *)memory::alloc_kpage(count);
        memset(arena, 0, count * PAGE_SIZE);
        arena->large = true;
        arena->count = count;
        arena->desc = nullptr;
        arena->magic = ONIX_MAGIC;

        addr = (char *)((uint32_t)arena + sizeof(arena_t));
        return addr;
    }
    for (size_t i = 0; i < DESC_COUNT; i++)
    {
        desc = &descriptors[i];
        if (desc->block_size >= size)
        {
            break;
        }
    }
    assert(desc != nullptr);

    if (desc->free_list.empty())
    {
        arena = (arena_t *)memory::alloc_kpage(1);
        memset(arena, 0, PAGE_SIZE);

        arena->desc = desc;
        arena->large = false;
        arena->count = desc->total_block;
        arena->magic = ONIX_MAGIC;

        for (size_t i = 0; i < desc->total_block; i++)
        {
            block = (block_t *)get_arena_block(arena, i);
            assert(!arena->desc->free_list.search(block));
            arena->desc->free_list.push_back(block);
            assert(arena->desc->free_list.search(block));
        }
    }

    block = desc->free_list.pop_front();
    arena = get_block_arena(block);
    assert(arena->magic == ONIX_MAGIC && !arena->large);

    arena->count--;
    return block;
}

void kfree(void *ptr)
{
    assert(ptr);

    block_t *block = (block_t *)ptr;
    arena_t *arena = get_block_arena(block);

    assert(arena->large == 1 || arena->large == 0);
    assert(arena->magic == ONIX_MAGIC);

    if (arena->large)
    {
        memory::free_kpage((uint32_t)arena, arena->count);
        return;
    }

    arena->desc->free_list.push_back(block);
    arena->count++;
    if (arena->count == arena->desc->total_block)
    { // 该 arena 所有块已被释放
        for (size_t i = 0; i < arena->desc->total_block; i++)
        {
            block = (block_t *)get_arena_block(arena, i);
            assert(arena->desc->free_list.search(block));
            List::remove(block);
            assert(!arena->desc->free_list.search(block));
        }
        memory::free_kpage((uint32_t)arena, 1);
    }
}
