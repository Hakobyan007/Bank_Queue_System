#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define PORT 8080
#define IP "127.0.0.1"
#include "./kiosk_headers/service_choice.h"
#include "./kiosk_headers/bank_services.h"

void queue_kiosk(){
    int fd = socket(AF_INET,SOCK_STREAM, 0);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = inet_addr(IP);

    if(connect(fd,(struct sockaddr*) &address,sizeof(address)) == -1){
        perror("connect");
        return;
    }
    const char kiosk_msg[10] = "REG_KIOSK";
    write(fd,kiosk_msg,strlen(kiosk_msg) + 1);

    while(1){

        int priority_type = -1;
        while(1) {
            system("clear");
            printf("\n");
            printf(" ╔═════════════════════════════════════╗\n");
            printf(" ║        CUSTOMER SERVICE MENU        ║\n");
            printf(" ╠═════════════════════════════════════╣\n");
            printf(" ║  [1] Natural Person                 ║\n");
            printf(" ║  [2] Legal Entity                   ║\n");
            printf(" ╚═════════════════════════════════════╝\n");
            printf(" ▶ Please select a service (1 or 2): ");
            
            
            if(scanf("%d", &priority_type) != 1) {
                printf("\033[1;31m   [!] Invalid input! Please enter a valid number.\033[0m\n");
                while(getchar() != '\n');
                continue;
            }
            else if(priority_type < 1 || priority_type > 2) {
                printf("\033[1;31m   [!] Invalid choice! Please select 1 or 2.\033[0m\n");
                continue;
            }
            
            printf("\n\033[1;32m   [✓] Service successfully selected -> %s\033[0m\n", 
                priority_type == 1 ? "Natural Person" : "Legal Entity");
                
            while(getchar() != '\n');
            break;
        }
        system("clear");
        printf("\n\033[1;32m   [✓] Service successfully selected -> %s\033[0m\n", 
                priority_type == 1 ? "Natural Person" : "Legal Entity");

        int service_index = service_choice();
        char server_msg[100] = {0};
        sprintf(server_msg,"%sP-%d",service_msgs[service_index],priority_type);
        write(fd, server_msg,strlen(server_msg) + 1);

        char server_answer[128] = "";
        int end = read(fd,server_answer,sizeof(server_answer));
        if(end > 0) server_answer[end] = '\0';

        if(!strncmp(server_answer,"Ticket:", strlen("Ticket:"))){
            printf("Your %s\n\n\n", server_answer);    
            printf("===================================\n");

        }
    }
}