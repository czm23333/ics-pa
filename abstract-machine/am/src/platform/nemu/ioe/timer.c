#include <am.h>
#include <nemu.h>

typedef union {
    uint64_t time;
    struct {
        uint32_t timeHigh, timeLow;
    };
} time_t;

time_t beginTime;

static void read_time(time_t* time) {
    time->timeLow = inl(RTC_ADDR);
    time->timeHigh = inl(RTC_ADDR + 4);
}

void __am_timer_init() {
    read_time(&beginTime);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
    time_t now;
    read_time(&now);
    uptime->us = (now.time - beginTime.time) / 1000;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
    rtc->second = 0;
    rtc->minute = 0;
    rtc->hour = 0;
    rtc->day = 0;
    rtc->month = 0;
    rtc->year = 1900;
}
