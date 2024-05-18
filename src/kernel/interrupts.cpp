#include <interrupts.h>
#include <stdio.h>
#include <io.h>
#include <assert.h>

#define ENTRY_SIZE 0x30
handler_t handler_table[IDT_SIZE];
extern handler_t handler_entry_table[ENTRY_SIZE];

static pointer_t idt_ptr;
static gate_t idt[IDT_SIZE];

extern "C" void inter_handler();
extern "C" void syscall_handler();

// 初始化中断描述符，和中断处理函数数组
void InterruptManager::idt_init()
{
    for (size_t i = 0; i < ENTRY_SIZE; i++)
    {
        gate_t *gate = &idt[i];
        handler_t handler = handler_entry_table[i];
        gate->offset0 = (uint32_t)handler & 0xffff;
        gate->offset1 = ((uint32_t)handler >> 16) & 0xffff;
        gate->selector = 1 << 3; // 代码段
        gate->reserved = 0;      // 保留不用
        gate->type = 0b1110;     // 中断门
        gate->segment = 0;       // 系统段
        gate->DPL = 0;           // 内核态
        gate->present = 1;       // 有效
    }

    for (size_t i = 0; i < 0x20; i++)
    {
        handler_table[i] = (handler_t)&InterruptManager::exception_handler;
    }

    for (size_t i = 0x20; i < ENTRY_SIZE; i++)
    {
        handler_table[i] = (handler_t)&InterruptManager::default_handler;
    }

    // 初始化系统调用
    gate_t *gate = &idt[0x80];
    gate->offset0 = (uint32_t)syscall_handler & 0xffff;
    gate->offset1 = ((uint32_t)syscall_handler >> 16) & 0xffff;
    gate->selector = 1 << 3; // 代码段
    gate->reserved = 0;      // 保留不用
    gate->type = 0b1110;     // 中断门
    gate->segment = 0;       // 系统段
    gate->DPL = 3;           // 用户态
    gate->present = 1;       // 有效

    idt_ptr.base = (uint32_t)idt;
    idt_ptr.limit = sizeof(idt) - 1;

    asm volatile("lidt %0" : : "m"(idt_ptr));

}

// 初始化中断控制器
void InterruptManager::pic_init()
{
    outb(PIC_M_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_M_DATA, 0x20);       // ICW2: 起始中断向量号 0x20
    outb(PIC_M_DATA, 0b00000100); // ICW3: IR2接从片.
    outb(PIC_M_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_S_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_S_DATA, 0x28);       // ICW2: 起始中断向量号 0x28
    outb(PIC_S_DATA, 2);          // ICW3: 设置从片连接到主片的 IR2 引脚
    outb(PIC_S_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_M_DATA, 0b11111111); // 关闭所有中断
    outb(PIC_S_DATA, 0b11111111); // 关闭所有中断
}

// 中断处理初始化
void InterruptManager::interrupt_init()
{
    pic_init();
    idt_init();
}

// 默认中断处理函数
void InterruptManager::default_handler(int vector)
{
    send_eoi(vector);
    DEBUG("[0x%X] default interrupt called...\n", vector);
    // set_interrupt_mask(vector-0x20, false);
}

// 默认异常处理函数
void InterruptManager::exception_handler(
    int vector,
    uint32_t edi, uint32_t esi, uint32_t ebp, uint32_t esp,
    uint32_t ebx, uint32_t edx, uint32_t ecx, uint32_t eax,
    uint32_t gs, uint32_t fs, uint32_t es, uint32_t ds,
    uint32_t vector0, uint32_t error, uint32_t eip, uint32_t cs, uint32_t eflags)
{
    char *message = NULL;
    if (vector < 22)
    {
        message = messages[vector];
    }
    else
    {
        message = messages[15];
    }

    printk("\nEXCEPTION : %s \n", message);
    printk("   VECTOR : 0x%02X\n", vector);
    printk("    ERROR : 0x%08X\n", error);
    printk("   EFLAGS : 0x%08X\n", eflags);
    printk("       CS : 0x%02X\n", cs);
    printk("      EIP : 0x%08X\n", eip);
    printk("      ESP : 0x%08X\n", esp);

    // bool hanging = true;

    // 阻塞
    while (true)
        ;
    // 通过 EIP 的值应该可以找到出错的位置
    // 也可以在出错时，可以将 hanging 在调试器中手动设置为 0
    // 然后在下面 return 打断点，单步调试，找到出错的位置
    return;
}

// 通知中断控制器，中断处理结束
void InterruptManager::send_eoi(int vector)
{
    if (vector >= 0x20 && vector < 0x28)
    {
        outb(PIC_M_CTRL, PIC_EOI);
    }
    if (vector >= 0x28 && vector < 0x30)
    {
        outb(PIC_M_CTRL, PIC_EOI);
        outb(PIC_S_CTRL, PIC_EOI);
    }
}

/**
 * @brief 注册中断处理函数
 * @param intNum 中断号码
 * @param handler 中断函数指针 
 */ 
void InterruptManager::set_interrupt_handler(int intNum, handler_t handler)
{
    assert(intNum >= 0 && intNum <= 17);
    handler_table[intNum + IRQ_MASTER_NR] = handler;
}

// 设置中断屏蔽字
void InterruptManager::set_interrupt_mask(int intNum, bool enable)
{
    assert(intNum >= 0 && intNum < 16);
    uint16_t port;
    if (intNum < 8)
    {
        port = PIC_M_DATA;
    }
    else
    {
        port = PIC_S_DATA;
    }
    if (enable)
    {
        outb(port, inb(port) & ~(1 << intNum));
    }
    else
    {
        outb(port, inb(port) | (1 << intNum));
    }
}

bool InterruptManager::disable_interrupt()
{
    asm volatile(
        "pushfl\n"        // 将当前 eflags 压入栈中
        "cli\n"           // 清除 IF 位，此时外中断已被屏蔽
        "popl %eax\n"     // 将刚才压入的 eflags 弹出到 eax
        "shrl $9, %eax\n" // 将 eax 右移 9 位，得到 IF 位
        "andl $1, %eax\n" // 只需要 IF 位
    );
}

/**
 * @brief 获取当前中断状态
 * @return - true 允许中断
 * @return - false 不允许中断 
*/
bool InterruptManager::get_interrupt_state()
{
    asm volatile(
        "pushfl\n"        // 将当前 eflags 压入栈中
        "popl %eax\n"     // 将压入的 eflags 弹出到 eax
        "shrl $9, %eax\n" // 将 eax 右移 9 位，得到 IF 位
        "andl $1, %eax\n" // 只需要 IF 位
    );
}

/**
 * @brief 打开中断或关闭中断
*/
void InterruptManager::set_interrupt_state(bool state)
{
    if (state)
        asm volatile("sti\n");
    else
        asm volatile("cli\n");
}
