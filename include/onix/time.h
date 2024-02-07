#pragma once
#include <types.h>
#include <io.h>

#define CMOS_ADDR 0x70 // CMOS 地址寄存器
#define CMOS_DATA 0x71 // CMOS 数据寄存器

// 下面是 CMOS 信息的寄存器索引
// 0x01 0x03 等留给实时时钟（闹钟）
#define CMOS_SECOND 0x00  // (0 ~ 59)
#define CMOS_MINUTE 0x02  // (0 ~ 59)
#define CMOS_HOUR 0x04    // (0 ~ 23)
#define CMOS_WEEKDAY 0x06 // (1 ~ 7) 星期天 = 1，星期六 = 7
#define CMOS_DAY 0x07     // (1 ~ 31)
#define CMOS_MONTH 0x08   // (1 ~ 12)
#define CMOS_YEAR 0x09    // (0 ~ 99)
#define CMOS_CENTURY 0x32 // 可能不存在
#define CMOS_NMI 0x80

#define MINUTE 60          // 每分钟的秒数
#define HOUR (60 * MINUTE) // 每小时的秒数
#define DAY (24 * HOUR)    // 每天的秒数
#define YEAR (365 * DAY)   // 每年的秒数，以 365 天算

// 每个月开始时的已经过去天数
static int month[13] = {
    0, // 这里占位，没有 0 月，从 1 月开始
    0,
    (31),
    (31 + 29),
    (31 + 29 + 31),
    (31 + 29 + 31 + 30),
    (31 + 29 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30),
    (31 + 29 + 31 + 30 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
    (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)};

static uint8_t month_day[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //平年 
static uint8_t Leap_month_day[12]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; //闰年


struct tm
{
    int tm_sec;   // 秒数 [0，59]
    int tm_min;   // 分钟数 [0，59]
    int tm_hour;  // 小时数 [0，59]
    int tm_mday;  // 1 个月的天数 [0，31]
    int tm_mon;   // 1 年中月份 [0，11]
    int tm_year;  // 从 1900 年开始的年数
    int tm_wday;  // 1 星期中的某天 [0，6] (星期天 =0)
    int tm_yday;  // 1 年中的某天 [0，365]
    int tm_isdst; // 夏令时标志
};

#define TIMEZONE (8) // 北京时区调整 UTC+8
#define FOURYEARDAY (365+365+365+366)  //4年一个周期内的总天数（1970~2038不存在2100这类年份，故暂不优化）
typedef uint32_t time_t;

class Time
{
private:
    Time();
    static time_t startup_time;
    static tm time;
    static int century;
    static int get_yday(tm* time);
    static bool is_leap_year(int year); // 判断一个年份是否是闰年
public:
    static void time_init();
    static void time_read_bcd(tm *time);
    static void time_read(tm *time);

    static void covUnixTimeStp2Beijing(uint32_t unixTime, tm* timeBeijing);
    static uint32_t covBeijing2UnixTimeStp(tm* beijingTime);
    
};
