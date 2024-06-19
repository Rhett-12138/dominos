#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <task_queue.h>

uint32_t memory::memory_base = 0; // 可用内存基地址
uint32_t memory::memory_size = 0; // 可用内存大小
uint32_t memory::total_pages = 0; // 所有内存页数
uint32_t memory::free_pages = 0;  // 空闲内存页数

uint32_t memory::start_page = 0;   // 可分配物理内存起始位置
uint8_t *memory::memory_map;       // 物理内存数组
uint32_t memory::memory_map_pages; // 物理内存数组

Bitmap memory::kernel_map;

void memory::memory_init(uint32_t magic, uint32_t addr)
{
    uint32_t count = 0;
    ards_t *ptr;

    // 如果是 onix loader 进入内核
    if (magic == ONIX_MAGIC)
    {
        count = *(uint32_t *)addr;
        ptr = (ards_t *)(addr + 4);
        for (size_t i = 0; i < count; i++, ptr++)
        {
            LOG("Memory base 0x%p, size 0x%p, type %d", (uint32_t)ptr->base, (uint32_t)ptr->size, (uint32_t)ptr->type);

            if (ptr->type == ZONE_VALID && ptr->size > memory_size)
            {
                memory_base = (uint32_t)ptr->base;
                memory_size = (uint32_t)ptr->size;
            }
        }
    }
    else
    {
        panic("Memory init magic unknown 0x%p", magic);
    }
    LOG("ARDS count %d", count);
    LOG("Memory base 0x%p", (uint32_t)memory_base);
    LOG("Memory size 0x%p", (uint32_t)memory_size);

    assert(memory_base == MEMORY_BASE); // 内存开始位置为1M
    assert((memory_size & 0xfff) == 0); // 要求按页对齐

    total_pages = IDX(memory_size) + IDX(MEMORY_BASE);
    free_pages = IDX(memory_size);

    LOG("Pages total:%d, free:%d", total_pages, free_pages);
}

/**
 * 分配一页物理内存
 */
uint32_t memory::get_page()
{
    for (size_t i = start_page; i < total_pages; i++)
    {
        // 如果有物理内存没有占用
        if (!memory_map[i])
        {
            memory_map[i] = 1;
            free_pages--;
            assert(free_pages >= 0);
            uint32_t page = ((uint32_t)i) << 12;
            LOG("Get page 0x%p", page);
            return page;
        }
    }
    panic("Out of Memory!!!");
}

// 将 cr0 寄存器最高位 PG 置为 1，启用分页
inline void memory::enable_page()
{
    // 0b1000_0000_0000_0000_0000_0000_0000_0000
    // 0x80000000
    asm volatile(
        "movl %cr0, %eax\n"
        "orl $0x80000000, %eax\n"
        "movl %eax, %cr0\n");
}

/**
 * 初始化页表项
 * @param entry 页表项指针
 * @param index 页索引，地址前20位
 */
void memory::entry_init(page_entry_t *entry, uint32_t index)
{
    *(uint32_t *)entry = 0;
    entry->present = 1;
    entry->write = 1;
    entry->user = 1;
    entry->index = index;
}

// 得到 cr3 寄存器
uint32_t memory::get_cr3()
{
    // 直接将 mov eax, cr3，返回值在 eax 中
    asm volatile("movl %cr3, %eax\n");
}

// 设置 cr3 寄存器，参数是页目录的地址
void memory::set_cr3(uint32_t pde)
{
    ASSERT_PAGE(pde);
    asm volatile("movl %%eax, %%cr3\n" ::"a"(pde));
}

void memory::mapping_init()
{
    page_entry_t *pde = (page_entry_t *)KERNEL_PAGE_DIR;
    memset(pde, 0, PAGE_SIZE);

    idx_t index = 0;

    for (idx_t didx = 0; didx < (sizeof(KERNEL_PAGE_TABLE) / 4); didx++)
    {
        page_entry_t *pte = (page_entry_t *)KERNEL_PAGE_TABLE[didx];
        memset(pte, 0, PAGE_SIZE);

        page_entry_t *dentry = &pde[didx];
        entry_init(dentry, IDX((uint32_t)pte));
        // dentry->user =

        for (size_t tidx = 0; tidx < 1024; tidx++, index++)
        {
            // 第 0 页不映射，为造成空指针访问，缺页异常，便于排错
            if (index == 0)
                continue;
            page_entry_t *tentry = &pte[tidx];
            entry_init(tentry, index);
            memory_map[index] = 1; // 设置物理内存数组，该页被占用
        }
    }

    // 将最后一个页表指向页目录自己，方便修改
    page_entry_t *entry = &pde[1023];
    entry_init(entry, IDX(KERNEL_PAGE_DIR));

    set_cr3((uint32_t)pde);

    enable_page();
}

/**
 * 获取页目录
 */
page_entry_t *memory::get_pde()
{
    // 页目录最后一页指向页目录自己
    return (page_entry_t *)(0xfffff000);
}

/**
 * 获取虚拟地址 vaddr 对应的页表
 * @param vaddr 虚拟地址
 * @param create 如果不存在则创建
 */
page_entry_t *memory::get_pte(uint32_t vaddr, bool create)
{
    page_entry_t *pde = get_pde();
    uint32_t idx = DIDX(vaddr);
    page_entry_t *entry = &pde[idx]; // 指向其页目录项，即页表入口

    assert(create || (!create && entry->present)); // 创建或该页表存在

    page_entry_t *table = (page_entry_t *)(PDE_MASK | (idx << 12)); 

    if(!entry->present)
    {
        LOG("Get and create page table entry for 0x%p", vaddr);
        uint32_t page = get_page();
        entry_init(entry, IDX(page));
        // 此处为什么不直接使用page进行memset，因为table也是一个地址，通过地址转换机构转换后访问到的还是page ？
        memset(table, 0, PAGE_SIZE);
    }
    return table;
}

// 刷新虚拟地址 vaddr 的 块表 TLB
void memory::flush_tlb(uint32_t vaddr)
{
    asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
}

void memory::memory_test()
{
    /*
    // 将 20M 0x1400000 内存映射到 64M 0x4000000 的位置
    // 还需要一个页表 0x900000

    uint32_t vaddr = 0x4000000;
    uint32_t paddr = 0x1400000;
    uint32_t table = 0x900000;

    page_entry_t* pde = get_pde();
    page_entry_t* dentry = &pde[DIDX(vaddr)];
    entry_init(dentry, IDX(table));

    page_entry_t* pte = get_pte(vaddr);
    page_entry_t* tentry = &pte[TIDX(vaddr)];

    entry_init(tentry, IDX(paddr));

    BMB;
    char* ptr = (char*)(vaddr);
    ptr[0] = 'a';

    BMB;
    entry_init(tentry, IDX(0x1500000));
    flush_tlb(vaddr);

    BMB;
    ptr[2] = 'b';
    */
    uint32_t *pages = (uint32_t *)(0x200000);
    uint32_t count = 0x6fe;
    for (size_t i = 0; i < count; i++)
    {
        pages[i] = alloc_kpage(1);
        LOG("0x%x", i);
    }
    for (size_t i = 0; i < count; i++)
    {
        free_kpage(pages[i], 1);
    }
}

/**
 * @brief 从位图中扫描并分配 count 个连续的页
 */
uint32_t memory::alloc_kpage(uint32_t count)
{
    assert(count > 0);
    uint32_t index = kernel_map.scan(count);
    if (index == EOF)
    {
        panic("Scan page fail!!!");
    }

    uint32_t addr = PAGE(index);
    LOG("ALLOC kernel page 0x%p count %d", addr, count);
    return addr;
}

void memory::free_kpage(uint32_t addr, uint32_t count)
{
    ASSERT_PAGE(addr);
    assert(count > 0);
    uint32_t index = addr >> 12;

    assert(kernel_map.test(index, count));
    kernel_map.set(index, 0, count);
    LOG("FREE kernel pages 0x%p count %d", addr, count);
}

/**
 * 将vaddr映射物理内存, vaddr应当为页起始地址
 * vaddr & 0xfff == 0
 */
void memory::link_page(uint32_t vaddr)
{
    ASSERT_PAGE(vaddr);

    page_entry_t* pte = get_pte(vaddr, true);
    page_entry_t* entry = &pte[TIDX(vaddr)];

    Task* task = TaskQueue::running_task();
    Bitmap* map = task->vmap;
    uint32_t index = IDX(vaddr);

    // 如果页面已存在，则直接返回
    if(entry->present)
    {
        assert(map->test(index));
        return;
    }

    assert(!map->test(index));
    map->set(index, true);

    uint32_t paddr = get_page();
    entry_init(entry, IDX(paddr));
    flush_tlb(vaddr);
    
    LOG("LINK from 0x%p to 0x%p", vaddr, paddr);
}

void memory::unlink_page(uint32_t vaddr)
{
    ASSERT_PAGE(vaddr);
    page_entry_t* pte = get_pte(vaddr, true);
    page_entry_t* entry = &pte[TIDX(vaddr)];

    Task* task = TaskQueue::running_task();
    Bitmap* map = task->vmap;
    uint32_t index = IDX(vaddr);

    // 如果页面已存在，则直接返回
    if(!entry->present)
    {
        assert(!map->test(index));
        return;
    }

    assert(entry->present&&map->test(index));
    entry->present = false;
    map->set(index, false);
    uint32_t paddr = PAGE(entry->index);
    LOG("UNLINK from 0x%p to 0x%p", vaddr, paddr);
    if(memory_map[entry->index] == 1)
    {
        free_page(paddr);
    }
    flush_tlb(vaddr);
}

void memory::free_page(uint32_t addr)
{
    ASSERT_PAGE(addr);

    uint32_t idx = IDX(addr);

    // idx 大于 1M 并且 小于 总页面数
    assert(idx >= start_page && idx < total_pages);

    // 保证只有一个引用
    assert(memory_map[idx] >= 1);

    // 物理引用减一
    memory_map[idx]--;

    // 若为 0，则空闲页加一
    if (!memory_map[idx])
    {
        free_pages++;
    }

    assert(free_pages > 0 && free_pages < total_pages);
    LOG("Free page 0x%p", addr);
}

void memory::memory_map_init()
{
    // 初始化物理内存数组
    memory_map = (uint8_t *)memory_base;

    // 计算物理内存占用的页数
    memory_map_pages = div_round_up(total_pages, PAGE_SIZE); // 1页可以管理4K页
    LOG("Memory map page count %d", memory_map_pages);

    free_pages -= memory_map_pages;

    // 清空物理内存数组
    memset((void *)memory_map, 0, memory_map_pages * PAGE_SIZE);

    // 前 1M 的内存位置 以及 物理内存数组已占用的页，已被占用
    start_page = IDX(MEMORY_BASE) + memory_map_pages;
    for (size_t i = 0; i < start_page; i++)
    {
        memory_map[i] = 1;
    }

    LOG("Total pages %d free pages %d", total_pages, free_pages);

    if (memory_size < KERNEL_MEMORY_SIZE)
    {
        panic("System memory is %dMB, too small, at least %dMD needed.", memory_size / MEMORY_BASE, KERNEL_MEMORY_SIZE / MEMORY_BASE);
    }
    // 初始化内核虚拟内存位图，需要 8 位对齐
    uint32_t length = (IDX(KERNEL_MEMORY_SIZE) - IDX(MEMORY_BASE)) / 8;
    kernel_map.init((char *)KERNEL_MAP_BITS, length, IDX(MEMORY_BASE));
    kernel_map.scan(memory_map_pages);
}
