#include <time.h>

#include <stdlib.h>
#include <stdio.h>

time_t Time::startup_time;
tm Time::time;
int Time::century;

uint8_t cmos_read(uint8_t addr)
{
    outb(CMOS_ADDR, CMOS_NMI | addr);
    return inb(CMOS_DATA);
}

int Time::get_yday(tm *time)
{
    int res = month[time->tm_mon]; // 已经过去的月的天数
    res += time->tm_mday;          // 这个月过去的天数

    int year;
    if (time->tm_year >= 70)
        year = time->tm_year - 70;
    else
        year = time->tm_year - 70 + 100;

    // 如果不是闰年，并且 2 月已经过去了，则减去一天
    // 注：1972 年是闰年，这样算不太精确，忽略了 100 年的平年
    if (is_leap_year(year) && time->tm_mon > 2)
    {
        res -= 1;
    }

    return res;
}

bool Time::is_leap_year(int year)
{
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
        return true;
    return false;
}

void Time::time_init()
{
    time_read(&time);
    startup_time = covBeijing2UnixTimeStp(&time);
    LOG("startup time: %d%d-%02d-%02d %02d:%02d:%02d",
        century,
        time.tm_year,
        time.tm_mon,
        time.tm_mday,
        time.tm_hour,
        time.tm_min,
        time.tm_sec);
}

void Time::time_read_bcd(tm *time)
{
    // CMOS 的访问速度很慢。为了减小时间误差，在读取了下面循环中所有数值后，
    // 若此时 CMOS 中秒值发生了变化，那么就重新读取所有值。
    // 这样内核就能把与 CMOS 的时间误差控制在 1 秒之内。
    do
    {
        time->tm_sec = cmos_read(CMOS_SECOND);
        time->tm_min = cmos_read(CMOS_MINUTE);
        time->tm_hour = cmos_read(CMOS_HOUR);
        time->tm_wday = cmos_read(CMOS_WEEKDAY);
        time->tm_mday = cmos_read(CMOS_DAY);
        time->tm_mon = cmos_read(CMOS_MONTH);
        time->tm_year = cmos_read(CMOS_YEAR);
        century = cmos_read(CMOS_CENTURY);
    } while (time->tm_sec != cmos_read(CMOS_SECOND));
}

void Time::time_read(tm *time)
{
    time_read_bcd(time);
    time->tm_sec = bcd_to_bin(time->tm_sec);
    time->tm_min = bcd_to_bin(time->tm_min);
    time->tm_hour = bcd_to_bin(time->tm_hour);
    time->tm_wday = bcd_to_bin(time->tm_wday);
    time->tm_mday = bcd_to_bin(time->tm_mday);
    time->tm_mon = bcd_to_bin(time->tm_mon);
    time->tm_year = bcd_to_bin(time->tm_year);
    time->tm_yday = get_yday(time);
    time->tm_isdst = -1;
    century = bcd_to_bin(century);

    
}

/**
 * @brief 将Unix时间戳转换为北京时间
 * @param unixTime Unix时间戳
 * @param *timeBeijing 返回的北京时间
 * @return none
 * @note 没有做输入的正确性检查
 */
void Time::covUnixTimeStp2Beijing(uint32_t unixTime, tm *timeBeijing)
{
    uint32_t totalDayNum = 0, totalSecNum = 0;
    uint16_t remainDayofYear = 0, tempDay = 0, tempYear = 0;
    uint8_t *pr = NULL;

    totalDayNum = unixTime / (24 * 60 * 60); // 总天数
    totalSecNum = unixTime % (24 * 60 * 60); // 当天剩余的秒数

    // 1.计算 HH:MM:SS
    timeBeijing->tm_hour = totalSecNum / 3600;
    timeBeijing->tm_min = (totalSecNum % 3600) / 60;
    timeBeijing->tm_sec = (totalSecNum % 3600) % 60;

    // 2.对时间进行时区调整（可能造成日期+1）
    timeBeijing->tm_hour += TIMEZONE;
    if (timeBeijing->tm_hour > 23)
    {
        timeBeijing->tm_hour -= 24;
        totalDayNum++; // 日期+1
    }

    // 3.计算年份
    timeBeijing->tm_year = 1970 + (totalDayNum / FOURYEARDAY) * 4;
    remainDayofYear = totalDayNum % FOURYEARDAY;

    tempYear = is_leap_year(timeBeijing->tm_year) ? 366 : 365;
    while (remainDayofYear > tempYear)
    {
        timeBeijing->tm_year++;
        remainDayofYear -= tempYear;
        tempYear = is_leap_year(timeBeijing->tm_year) ? 366 : 365;
    }

    // 4.计算月份和几号
    pr = is_leap_year(timeBeijing->tm_year) ? Leap_month_day : month_day;
    remainDayofYear++; // 这里开始计算具体日期。remainDayofYear为 0 时其实是 1 号，所以这里要 +1
    timeBeijing->tm_mon = 0;
    while (remainDayofYear > pr[timeBeijing->tm_mon])
    {
        remainDayofYear -= pr[timeBeijing->tm_mon];
        timeBeijing->tm_mon++;
    }
    timeBeijing->tm_mon++;
    timeBeijing->tm_mday = remainDayofYear;
}

/**
 * @brief 将北京时间转换为Unix时间戳
 * @param *beijingTime 北京时间
 * @return Unix时间戳(从1970/1/1 00:00:00到现在的秒数)
 * @note 没有对输入参数的正确性做判断
 */
uint32_t Time::covBeijing2UnixTimeStp(tm *beijingTime)
{
    uint32_t dayNum = 0, secNum = 0;         // 保存北京时间到起始时间的天数
    uint16_t tempYear = 1970, tempMonth = 0; // 起始时间

    // 1.年的天数
    while (tempYear < beijingTime->tm_year)
    {
        if (is_leap_year(tempYear))
        {
            dayNum += 366;
        }
        else
        {
            dayNum += 365;
        }
        tempYear++;
    }

    // 2.月的天数
    dayNum += month[beijingTime->tm_mon];
    if (is_leap_year(beijingTime->tm_year) && beijingTime->tm_mon > 2)
    {
        dayNum++;
    }

    // 3.天数
    dayNum += (beijingTime->tm_mday - 1);

    // 4.时分秒
    secNum = dayNum * 24 * 60 * 60; // s
    secNum += beijingTime->tm_hour * 60 * 60;
    secNum += beijingTime->tm_min * 60;
    secNum += beijingTime->tm_sec;

    // 时区调整
    secNum -= TIMEZONE * 60 * 60;

    return secNum;
}
