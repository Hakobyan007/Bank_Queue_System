#include <time.h>
#include <string.h>
#include <stdio.h>

void time_for_tickets(char* time_result,char* date){
    time_t current_time = time(NULL);
    struct tm* time_info;
    time_info = localtime(&current_time);
    sprintf(time_result,"time -> %02d:%02d", time_info->tm_hour,time_info->tm_min);
    sprintf(date,"date -> %02d/%02d/%d",time_info->tm_mday,time_info->tm_mon + 1,time_info->tm_year + 1900);
}