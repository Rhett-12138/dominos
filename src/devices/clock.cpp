#include <clock.h>

#include <task_queue.h>

uint32_t volatile Clock::jiffies;
uint32_t Clock::jiffy;
uint32_t volatile Clock::beeping;

Clock::Clock(int num):InterruptHandler(num)
{
    pit_init();
    jiffies = 0;
    jiffy = JIFFY;
    InterruptManager::set_interrupt_handler(num, (handler_t)&Clock::handler);
    LOG("CLOCK INIT.");
    enable();
    
}

Clock::~Clock()
{
}

void Clock::handler(int vector)
{
    // LOG("vector: 0x%x", vector);
    assert(vector == 0x20);
    InterruptManager::send_eoi(vector);
    // stop_beep();

    jiffies++;
    // LOG("clock jiffies %d ...", jiffies);
    
    Task* task = TaskQueue::running_task();
    assert(task->magic==ONIX_MAGIC);

    task->jiffies = jiffies;
    TaskQueue::task_wakeup();
    task->ticks--;
    if(!task->ticks)
    {   // 时间片完
        task->ticks = task->priority;
        TaskQueue::schedule();
    }
}

void Clock::start_beep()
{
    if(!beeping)
    {
        outb(SPEAKER_REG, inb(SPEAKER_REG) | 0b11);
        // beeping = true;

        // task_sleep(BEEP_MS);

        // outb(SPEAKER_REG, inb(SPEAKER_REG) & 0xfc);
        // beeping = false;
    }
    beeping = jiffies + 5;
}

void Clock::stop_beep()
{
    if(beeping&&jiffies>beeping)
    {
        outb(SPEAKER_REG, inb(SPEAKER_REG)&0xfc);
        beeping = 0;
    }
}

uint32_t Clock::get_jiffies()
{
    return jiffies;
}

void Clock::pit_init()
{
    // 配置计数器 0 时钟
    outb(PIT_CTRL_REG, 0b00110100);
    outb(PIT_CHAN0_REG, CLOCK_COUNTER & 0xff);
    outb(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff);

    // 配置计数器 2 蜂鸣器
    outb(PIT_CTRL_REG, 0b10110110);
    outb(PIT_CHAN2_REG, (uint8_t)BEEP_COUNTER);
    outb(PIT_CHAN2_REG, (uint8_t)(BEEP_COUNTER >> 8));
}
