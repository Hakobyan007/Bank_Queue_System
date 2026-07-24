#include <stdio.h>
#include "../kiosk_headers/bank_services.h"
#define AVAILABLE_SERVICE_LIST_LENGTH 7

int service_choice(){
    printf("===================================\n");

    for(int i = 0;i < AVAILABLE_SERVICE_LIST_LENGTH; ++i){
        printf("%d. %s\n",i + 1, services[i]);
    }
    int service_choosen = 0;
    while(1){
        printf("Please select a service -> ");
        if(scanf("%d",&service_choosen) != 1){
            printf("Invalid input! Please enter a number.\n");
            while(getchar() != '\n');
            continue;
        }
        else if(service_choosen < 1 || service_choosen > AVAILABLE_SERVICE_LIST_LENGTH){
            printf("Invalid choice! Please select a number between 1 and %d.\n",AVAILABLE_SERVICE_LIST_LENGTH);
            continue;
        }
        printf("You select service -> %s\n", services[service_choosen - 1]);
        while(getchar() != '\n');
        break;
    }

    return service_choosen - 1;
}