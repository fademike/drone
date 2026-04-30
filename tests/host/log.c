

#include <stdint.h>
#include <sys/time.h>
#include "stdio.h"
#include "log.h"


void log_print_time(void)
{
    struct timeval start_tv;
    gettimeofday(&start_tv, NULL);
    uint32_t time_ms = start_tv.tv_sec;

//   sprintf(time_str, "[%02d:%02d.%03d] ", (time_ms/(60*1000))%60, (time_ms/1000)%60, time_ms%1000);
    printf("[%02d:%02d.%03d] ", (int)(start_tv.tv_sec/60)%60, (int)start_tv.tv_sec%60, (int)start_tv.tv_usec/1000);
}

void log_print(char )
{
    struct timeval start_tv;
    gettimeofday(&start_tv, NULL);
    uint32_t time_ms = start_tv.tv_sec;

//   sprintf(time_str, "[%02d:%02d.%03d] ", (time_ms/(60*1000))%60, (time_ms/1000)%60, time_ms%1000);
    printf("[%02d:%02d.%03d] ", (int)(start_tv.tv_sec/60)%60, (int)start_tv.tv_sec%60, (int)start_tv.tv_usec/1000);
}