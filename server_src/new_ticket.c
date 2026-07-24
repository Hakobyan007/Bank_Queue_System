#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../kiosk_headers/bank_services.h"
#define AVAILABLE_SERVICE_LIST_LENGTH 7

void new_ticket(char* category, char* result_store, int* service_queue){
    int service_index = -1;
    for(int i = 0; i < AVAILABLE_SERVICE_LIST_LENGTH; ++i){
        if(!strcmp(category,service_msgs[i])){
            service_index = i;
        }
    }  
    if(service_index < 0 || service_index > AVAILABLE_SERVICE_LIST_LENGTH) return;
    
    
    ++service_queue[service_index];
    sprintf(result_store,"%c-%d",services_letters[service_index],service_queue[service_index]);
}