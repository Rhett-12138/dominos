#include <io.h>
#include <interrupts.h>
#include <assert.h>
#include <stdio.h>

#define PIT_CHAN0_REG 0x40
#define PIT_CHAN2_REG 0x42
#define PIT_CTRL_REG 0x43

/**
 * 经过 CLOCK_COUNTER 发生一个中断
 * OSCILLATOR 默认频率
 * HZ 时钟频率
 * JIFFY 一个中断10ms
*/
#define HZ 100
#define OSCILLATOR 1193182
#define CLOCK_COUNTER (OSCILLATOR / HZ)
#define JIFFY (1000/HZ)

#define SPEAKER_REG 0x61
#define BEEP_HZ 1000
#define BEEP_COUNTER (OSCILLATOR / BEEP_HZ)
#define BEEP_MS 100

class Clock : public InterruptHandler
{
private:
    // 时间片计数器
    static uint32_t volatile jiffies;
    static uint32_t jiffy;
    static uint32_t volatile beeping;
public:
    Clock(int num = IRQ_CLOCK);
    ~Clock();
    static void handler(int vector);
    static void start_beep();
    static void stop_beep();
    void pit_init();
};





