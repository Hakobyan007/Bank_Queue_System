#ifndef QUEUE_COMMON
#define QUEUE_COMMON


typedef struct 
{
    char ticket_place[7];
    int service_index;
    int status;
    int priority;
    int window_num;
    char create_time[20];
    char create_date[20];
} ticket_example;

typedef struct
{
    int window_number;
    int admin_fd;
    int allowed_services[7];
    int status;
} window_example;


#endif