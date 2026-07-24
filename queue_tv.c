#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define PORT 8080
#define IP "127.0.0.1"
#define MAX_WINDOW_COUNT 15
#define MAX_MSG_LENGTH 100

void queue_tv(){
    int fd = socket(AF_INET,SOCK_STREAM,0);
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = inet_addr(IP); 

    if(connect(fd,(struct sockaddr*) &address,sizeof(address)) == -1){
        perror("connect");
        return;
    }

    char server_msg[10] = "REG_TV";
    write(fd,server_msg,strlen(server_msg) + 1);
    char show_tv_all[MAX_WINDOW_COUNT][MAX_MSG_LENGTH] = {0};

    while(1){
        char server_reply[MAX_MSG_LENGTH] = "";
        int end = read(fd,server_reply,sizeof(server_reply));
         
        if(end > 0){
            server_reply[end] = '\0';
            if(!strncmp(server_reply,"DISPLAY",strlen("DISPLAY"))){
                for(int i = 0;i < MAX_WINDOW_COUNT; ++i){
                    if(show_tv_all[i][0] == '\0'){
                        strcpy(show_tv_all[i], server_reply);
                        break;
                    }
                }
            }
            else if(!strncmp(server_reply,"DELETE",strlen("DELETE"))){
                for(int i = 0;i < MAX_WINDOW_COUNT; ++i){
                    if(strcmp(show_tv_all[i] + strlen("DISPLAY"),server_reply + strlen("DELETE"))){
                        strcpy(show_tv_all[i], "");
                        break;
                    }
                }
            }
            system("clear");
            for(int show = 0;show < MAX_WINDOW_COUNT; ++show){
                if(show_tv_all[show][0] == '\0'){
                    continue;
                }
                printf("===================================\n");
                printf("        %s\n", show_tv_all[show]);
                printf("===================================\n");
            }
        }
        else if(end == 0){
            printf("Server disconnected!");
            break;
        }
    }
    
}