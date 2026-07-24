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
        int service_index = service_choice();
        write(fd, service_msgs[service_index],strlen(service_msgs[service_index]) + 1);

        char server_answer[128] = "";
        int end = read(fd,server_answer,sizeof(server_answer));
        if(end > 0) server_answer[end] = '\0';

        if(!strncmp(server_answer,"Ticket:", strlen("Ticket:"))){
            printf("Your %s\n\n\n", server_answer);    
            printf("===================================\n");

        }
    }
}