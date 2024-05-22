#pragma once
#include <types.h>
#include <bitmap.h>

#define PAGE_SIZE 0x1000     // 一页的大小 4KB
#define MEMORY_BASE 0x100000 // 1MB，可用内存的起始位置

// 内核占用的内存大小 16M
// #define KERNEL_MEMORY_SIZE 0x1000000

// 内存虚拟磁盘地址
#define KERNEL_RAMDISK_MEM 0xC00000

// 内存虚拟磁盘大小
#define KERNEL_RAMDISK_SIZE 0x400000

// 用户程序地址
#define USER_EXEC_ADDR KERNEL_MEMORY_SIZE

// 用户映射内存开始位置
#define USER_MMAP_ADDR 0x8000000

// 用户映射内存大小
#define USER_MMAP_SIZE 0x8000000

// 用户栈顶地址 256M
#define USER_STACK_TOP 0x10000000

// 用户栈最大 2M
#define USER_STACK_SIZE 0x200000

// 用户栈底地址
#define USER_STACK_BOTTOM (USER_STACK_TOP - USER_STACK_SIZE)

#define ZONE_VALID 1    // ards 可用内存区域
#define ZONE_RESERVED 2 // ards 不可用区域

#define IDX(addr) ((uint32_t)addr >> 12)            // 获取addr的页索引
#define DIDX(addr) (((uint32_t)addr >> 22) & 0x3ff) // 获取 addr 的页目录索引
#define TIDX(addr) (((uint32_t)addr >> 12) & 0x3ff) // 获取 addr 的页表索引
#define PAGE(idx) ((uint32_t)idx << 12)             // 获取页索引 idx 对应的页开始的位置
#define PAGE(idx) ((uint32_t)idx << 12)             // 获取页索引idx 对应的页开始的位置
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

#define KERNEL_PAGE_DIR 0x1000     // 内核页目录
#define KERNEL_PAGE_ENTRY 0x201000 // 内核页表

#define KERNEL_MAP_BITS 0x4000

#define PDE_MASK 0xFFC00000

// 内核页表索引
static uint32_t KERNEL_PAGE_TABLE[] = {
    0x2000,
    0x3000};

#define KERNEL_MEMORY_SIZE (0x100000 * sizeof(KERNEL_PAGE_TABLE)) //

struct ards_t
{
    uint64_t base; // 内存基地址
    uint64_t size; // 内存长度
    uint32_t type; // 类型
} _packed;

// 32bit
struct page_entry_t
{
    uint8_t present : 1;  // 在内存中
    uint8_t write : 1;    // 0 只读 1 可读可写
    uint8_t user : 1;     // 1 所有人 0 超级用户 DPL < 3
    uint8_t pwt : 1;      // page write through 1 直写模式，0 回写模式
    uint8_t pcd : 1;      // page cache disable 禁止该页缓冲
    uint8_t accessed : 1; // 被访问过，用于统计使用频率
    uint8_t dirty : 1;    // 脏页，表示该页缓冲被写过
    uint8_t pat : 1;      // page attribute table 页大小 4K/4M
    uint8_t global : 1;   // 全局，所有进程都用到了，该页不刷新缓冲
    uint8_t shared : 1;   // 共享内存页，与 CPU 无关
    uint8_t privat : 1;   // 私有内存页，与 CPU 无关
    uint8_t readonly : 1; // 只读内存页，与 CPU 无关
    uint32_t index : 20;  // 页索引
} _packed;

#define used_pages (total_pages - free_pages) // 已用页数

class memory
{
private:
    static uint32_t memory_base; // 可用内存基地址
    static uint32_t memory_size; // 可用内存大小
    static uint32_t total_pages; // 所有内存页数
    static uint32_t free_pages;  // 空闲内存页数

    static uint32_t start_page;       // 可分配物理内存起始位置
    static uint8_t *memory_map;       // 物理内存数组
    static uint32_t memory_map_pages; // 物理内存数组占用的页数
    static Bitmap kernel_map;

private:
    memory();
    static void enable_page();                                   // 将 cr0 寄存器最高位 PG 置为 1，启用分页
    static void entry_init(page_entry_t *entry, uint32_t index); // 初始化页表项
public:
    static void memory_map_init();
    static void memory_init(uint32_t magic, uint32_t addr);
    static uint32_t get_page();           // 分配一页物理内存
    static void free_page(uint32_t addr); // 释放一页物理内存

    static uint32_t get_cr3();         // 得到 cr3 寄存器
    static void set_cr3(uint32_t pde); // 设置 cr3 寄存器，参数是页目录的地址
    static void mapping_init();

    static page_entry_t *get_pde();               // 获取页目录
    static page_entry_t *get_pte(uint32_t vaddr, bool create); // 获取虚拟地址 vaddr 对应的页表
    static void flush_tlb(uint32_t vaddr);        // 刷新虚拟地址 vaddr 的 块表 TLB
    static void memory_test();                    // TODO delete

    static uint32_t alloc_kpage(uint32_t count);             // 从位图中扫描并分配 count 个连续的页
    static void free_kpage(uint32_t addr, uint32_t count); // 与 scan_page 相对，重置相应的页
};
