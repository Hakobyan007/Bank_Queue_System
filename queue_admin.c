#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <stdbool.h>
#define PORT 8080
#define IP "127.0.0.1"
#define KEY_ENT 10   // Enter
#define KEY_ZERO '0' // 0 -> in ASCII 48
#define KEY_ONE '1'  // 1 -> in ASCII 49
#define KEY_TWO '2'  // 2 -> in ASCII 50


void queue_admin(){
    int fd = socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = inet_addr(IP);

    if(connect(fd,(struct sockaddr*) &address,sizeof(address)) == -1){
        perror("connect");
        return;
    }
    
    int window_num = 0;
    while(1){       
        printf("\nEnter your Window Number (1-15): ");
        if(scanf("%d", &window_num) != 1){
            printf("Invalid Input\n");
            while(getchar() != '\n');
            continue;
        }   
        else if(window_num < 1 || window_num > 15){
            printf("Please enter a number between 1 and 15");
            while(getchar() != '\n');
            continue;            
        }

        while(getchar() != '\n');
        break;
    }

    char adm_msg[20];
    sprintf(adm_msg,"REG_ADMIN: %d",window_num);
    write(fd,adm_msg,strlen(adm_msg) + 1);


    initscr();
    cbreak();
    noecho();
    keypad(stdscr,TRUE);
    char last_status[100] = "Waiting for action...";
    char last_ch = 0;
    bool is_called_customer = false;
    bool is_start_serving = false;
    
    
    while(1){   
        clear();

        printw("==============================================\n");
        printw("           BANK QUEUE ADMIN PANEL             \n");
        printw("==============================================\n");
        printw("   [ Active Window : %2d ]\n", window_num);
        printw("   STATUS: %s\n\n", last_status);
        if(!is_called_customer){
            printw("   [ ENTER ]  -  Call Next Customer\n");
            printw("   [   0   ]  -  Exit System\n");
        }
        else{
            printw("   [   1   ]  -  Mark as 'Serving' (Arrived)\n");
            printw("   [   2   ]  -  End Serving (Clear Window)\n");
        }
        printw("----------------------------------------------\n\n");
        printw(last_ch == 0 ? " You Marked -> NOTHING" : last_ch == 10 ? " You Marked -> ENTER" : " You Marked -> %c", last_ch);

        refresh();
        int ch = getch();
        if(ch == KEY_ENT && !is_called_customer){
            const char msg_to_server[15] = "CMD_NEXT";
            write(fd,msg_to_server,strlen(msg_to_server) + 1);
            
            char msg_from_server[100] = "";
            int end = read(fd,msg_from_server,sizeof(msg_from_server) - 1);
            if(end > 0){
                msg_from_server[end] = '\0';
            }
            if(!strcmp(msg_from_server, "EMPTY")){
                strcpy(last_status, "Queue is EMPTY");
                is_called_customer = false;
                is_start_serving = false;
            }
            else if(!strncmp(msg_from_server,"SERVING:",strlen("SERVING:"))){
                strcpy(last_status, msg_from_server);
                is_called_customer = true;
                is_start_serving = false;
            }
            last_ch = ch;
        }
        else if(ch == KEY_ONE && is_called_customer){   
            const char msg_to_server[15] = "CMD_SERVING";
            write(fd,msg_to_server,strlen(msg_to_server) + 1);
            strcpy(last_status,"HAS BEEN SENT");
            last_ch = ch;
            is_start_serving = true;
        }
        else if(ch == KEY_TWO && is_called_customer){
            if(!is_start_serving){
                const char msg_to_server[15] = "CMD_SERVING";
                write(fd,msg_to_server,strlen(msg_to_server) + 1);
                usleep(50000);
            }
            const char msg_to_server[15] = "CMD_SERVED";
            write(fd,msg_to_server,strlen(msg_to_server) + 1);
            strcpy(last_status,"HAS BEEN SENT");
            last_ch = ch;
            is_called_customer = false;
        }
        else if(ch == KEY_ZERO && ! is_called_customer){
            const char msg_to_server[15] = "CMD_EXIT";
            write(fd,msg_to_server,strlen(msg_to_server) + 1);
            strcpy(last_status,"HAS BEEN SENT");
            last_ch = ch;
            endwin();
            close(fd);
            break;
        }
        else{
            last_ch = 0;
            if(ch == ERR){
                printw("Invalid Key: Please Try Again...");
                refresh();
            }
        }

    }
}
