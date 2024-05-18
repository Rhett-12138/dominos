#pragma once
#include <types.h>
#include <global.h>
#include <io.h>

#define IDT_SIZE 256

#define INTR_DE 0   // 除零错误
#define INTR_DB 1   // 调试
#define INTR_NMI 2  // 不可屏蔽中断
#define INTR_BP 3   // 断点
#define INTR_OF 4   // 溢出
#define INTR_BR 5   // 越界
#define INTR_UD 6   // 指令无效
#define INTR_NM 7   // 协处理器不可用
#define INTR_DF 8   // 双重错误
#define INTR_OVER 9 // 协处理器段超限
#define INTR_TS 10  // 无效任务状态段
#define INTR_NP 11  // 段无效
#define INTR_SS 12  // 栈段错误
#define INTR_GP 13  // 一般性保护异常
#define INTR_PF 14  // 缺页错误
#define INTR_RE1 15 // 保留
#define INTR_MF 16  // 浮点异常
#define INTR_AC 17  // 对齐检测
#define INTR_MC 18  // 机器检测
#define INTR_XM 19  // SIMD 浮点异常
#define INTR_VE 20  // 虚拟化异常
#define INTR_CP 21  // 控制保护异常

#define IRQ_CLOCK 0      // 时钟
#define IRQ_KEYBOARD 1   // 键盘
#define IRQ_CASCADE 2    // 8259 从片控制器
#define IRQ_SERIAL_2 3   // 串口 2
#define IRQ_SERIAL_1 4   // 串口 1
#define IRQ_PARALLEL_2 5 // 并口 2
#define IRQ_SB16 5       // SB16 声卡
#define IRQ_FLOPPY 6     // 软盘控制器
#define IRQ_PARALLEL_1 7 // 并口 1
#define IRQ_RTC 8        // 实时时钟
#define IRQ_REDIRECT 9   // 重定向 IRQ2
#define IRQ_NIC 11       // 网卡
#define IRQ_MOUSE 12     // 鼠标
#define IRQ_MATH 13      // 协处理器 x87
#define IRQ_HARDDISK 14  // ATA 硬盘第一通道
#define IRQ_HARDDISK2 15 // ATA 硬盘第二通道

#define IRQ_MASTER_NR 0x20 // 主片起始向量号
#define IRQ_SLAVE_NR 0x28  // 从片起始向量号

#define PIC_M_CTRL 0x20 // 主片的控制端口
#define PIC_M_DATA 0x21 // 主片的数据端口
#define PIC_S_CTRL 0xa0 // 从片的控制端口
#define PIC_S_DATA 0xa1 // 从片的数据端口
#define PIC_EOI 0x20    // 通知中断控制器中断结束

struct gate_t
{
    uint16_t offset0;    // 段内偏移 0 ~ 15 位
    uint16_t selector;   // 代码段选择子
    uint8_t reserved;    // 保留不用
    uint8_t type : 4;    // 任务门/中断门/陷阱门
    uint8_t segment : 1; // segment = 0 表示系统段
    uint8_t DPL : 2;     // 使用 int 指令访问的最低权限
    uint8_t present : 1; // 是否有效
    uint16_t offset1;    // 段内偏移 16 ~ 31 位
} _packed;

typedef void *handler_t; // 中断处理函数

// class InterruptManager;

// class InterruptHandler
// {
// protected:
//     uint8_t intNumber;
//     InterruptManager* intManager;
//     InterruptHandler(InterruptManager* interruptManager, uint8_t interruptNumber);
//     ~InterruptHandler();
// public:
//     virtual uint32_t HandleInterrupt(uint32_t esp);

// };

static char *messages[] = {
    "#DE Divide Error\0",
    "#DB RESERVED\0",
    "--  NMI Interrupt\0",
    "#BP Breakpoint\0",
    "#OF Overflow\0",
    "#BR BOUND Range Exceeded\0",
    "#UD Invalid Opcode (Undefined Opcode)\0",
    "#NM Device Not Available (No Math Coprocessor)\0",
    "#DF Double Fault\0",
    "    Coprocessor Segment Overrun (reserved)\0",
    "#TS Invalid TSS\0",
    "#NP Segment Not Present\0",
    "#SS Stack-Segment Fault\0",
    "#GP General Protection\0",
    "#PF Page Fault\0",
    "--  (Intel reserved. Do not use.)\0",
    "#MF x87 FPU Floating-Point Error (Math Fault)\0",
    "#AC Alignment Check\0",
    "#MC Machine Check\0",
    "#XF SIMD Floating-Point Exception\0",
    "#VE Virtualization Exception\0",
    "#CP Control Protection Exception\0",
};

static void clock_int(int vector);

class InterruptManager
{
private:
    InterruptManager();

    static void idt_init(); // 初始化中断描述符，和中断处理函数数组
    static void  pic_init(); // 初始化中断控制器

public:
    static void interrupt_init();

    static void HandleInterrupt();
    static void default_handler(int vector); // 默认中断处理函数
    static void exception_handler(
        int vector,
        uint32_t edi, uint32_t esi, uint32_t ebp, uint32_t esp,
        uint32_t ebx, uint32_t edx, uint32_t ecx, uint32_t eax,
        uint32_t gs, uint32_t fs, uint32_t es, uint32_t ds,
        uint32_t vector0, uint32_t error, uint32_t eip, uint32_t cs, uint32_t eflags); // 默认异常处理
    static void send_eoi(int vector);                                 // 通知中断控制器，中断处理结束
    static void set_interrupt_handler(int intNum, handler_t handler); // 注册中断处理函数
    static void set_interrupt_mask(int intNum, bool enable);          // 设置中断屏蔽字

    static bool disable_interrupt();             // 清除 IF 位， 返回之前的值
    static bool get_interrupt_state();           // 获得 IF 位
    static void set_interrupt_state(bool state); // 设置 IF 位
};