    #include <stdio.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <sys/select.h>
    #include <unistd.h>
    #include <string.h>
    #include <time.h>
    #include <mysql.h>
    #include "queue_common.h"
    #include "./server_headers/time_for_tickets.h"
    #include "./server_headers/new_ticket.h"
    #include "./kiosk_headers/bank_services.h"
    #define PORT 8080
    #define AVAILABLE_SERVICE_LIST_LENGTH 7
    #define WINDOWS_COUNT 15
    #define MAX_CLIENTS 20
    #define QUEUE_MAX 100



    void queue_server(){

        // Data Base Working
        
        MYSQL *conn = mysql_init(NULL);

        if (conn == NULL) {
            perror("mysql initialization");   
            return;
        }

        if(mysql_real_connect(conn, "localhost", "root", "123321", "bank_db", 3306, NULL, 0) == NULL){
            perror("Connection error");
            return;
        }
        if (mysql_set_character_set(conn, "utf8mb4")) {
            fprintf(stderr, "Character set error: %s\n", mysql_error(conn));
        }
        const char *sql = "CREATE TABLE IF NOT EXISTS bank_tickets ("
                    "id INT AUTO_INCREMENT PRIMARY KEY, "
                    "ticket_place VARCHAR(7), "
                    "service_index INT, "
                    "priority INT, "
                    "create_time VARCHAR(20), "
                    "create_date VARCHAR(20)"
                    ")";
        if(mysql_query(conn, sql)){
            perror("Table creation error");
            return;
        }

        //


        
        int fd = socket(AF_INET,SOCK_STREAM,0);
        
        int opt = 1;
        if(setsockopt(fd,SOL_SOCKET, SO_REUSEADDR,&opt,sizeof(opt)) < 0){
            perror("setsockopt failed");
            return;
        }

        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(PORT);
        address.sin_addr.s_addr = INADDR_ANY;

        if(bind(fd, (struct sockaddr*) &address,sizeof(address)) < 0){
            perror("bind");
            return;
        }

        if(listen(fd, 3)){
            perror("listen");
            return;
        }

        fd_set read_fds;
        
        int client_sockets[MAX_CLIENTS] = {0};


        int version1[7] = {1, 0, 0, 0, 0, 1, 1}; 
        int version2[7] = {0, 0, 0, 1, 1, 0, 0}; 
        int version3[7] = {0, 1, 1, 0, 0, 0, 0}; 
        int version4[7] = {1, 1, 1, 1, 1, 1, 1}; 

        window_example windows[15];

        for (int i = 0; i < WINDOWS_COUNT; ++i) {
            windows[i].window_number = i + 1;
            windows[i].admin_fd = -1;
            windows[i].status = 0;

            if (i < 5)         memcpy(windows[i].allowed_services, version1, sizeof(version1));
            else if (i < 10)   memcpy(windows[i].allowed_services, version2, sizeof(version2));
            else if (i < 12)   memcpy(windows[i].allowed_services, version3, sizeof(version3));
            else               memcpy(windows[i].allowed_services, version4, sizeof(version4));
        }


        ticket_example queue[QUEUE_MAX];
        int ticket_count = 0;
        int current_index = 0;
        
        int max_fd = fd;
        
        int kiosk_fd = -1;
        int tv_fd = -1;

        int service_queue[AVAILABLE_SERVICE_LIST_LENGTH] = {0};
        


        
        


        while (1)
        {
            FD_ZERO(&read_fds);
            FD_SET(fd, &read_fds);
            
            for(int i = 0 ; i < MAX_CLIENTS ; ++i){
                if(client_sockets[i] > 0){
                    FD_SET(client_sockets[i],&read_fds);
                }
            }
            struct timeval timer = {.tv_sec = 5, .tv_usec = 0};
            int activity = select(max_fd + 1, &read_fds,NULL,NULL,&timer);
            if(activity == -1){
                perror("select");
                break;
            }
            else if(activity == 0){
                printf("nothing new...\n");
                fflush(stdout);
                continue;
            }
            if(FD_ISSET(fd,&read_fds)){
                int client_fd = accept(fd,NULL,NULL);
                if(client_fd == -1){
                    perror("accept");
                    break;
                }
                else if(client_fd >= 0){
                    printf("kpela...\n");
                    fflush(stdout);
                    for (int i = 0; i < MAX_CLIENTS; i++)
                    {
                        if(client_sockets[i] != 0) continue;
                        client_sockets[i] = client_fd;
                        if(client_fd > max_fd) max_fd = client_fd;
                        break;
                    }                
                }
            }
            for(int i = 0; i < MAX_CLIENTS; ++i){
                if(client_sockets[i] > 0 && FD_ISSET(client_sockets[i],&read_fds)){
                    char buffer[1024];
                    int read_res = read(client_sockets[i], buffer, sizeof(buffer));
                    if(read_res == 0){
                        close(client_sockets[i]);
                        for(int adm = 0; adm < WINDOWS_COUNT; ++adm){
                            if(windows[adm].admin_fd == client_sockets[i]){
                                windows[adm].admin_fd = -1;               
                            }
                        }
                        client_sockets[i] = 0;
                        max_fd = fd;
                        for(int j = 0;j < MAX_CLIENTS; ++j){
                            if(max_fd < client_sockets[j]) max_fd = client_sockets[j];
                        }
                    }
                    else if(read_res == -1){
                        perror("read");
                        fflush(stdout);
                        continue;
                    }
                    else{
                        buffer[read_res] = '\0';
                        if(!strcmp(buffer,"REG_KIOSK")) kiosk_fd = client_sockets[i];
                        else if(!strncmp(buffer,"REG_ADMIN",strlen("REG_ADMIN"))) {
                            int id = -1;
                            if(sscanf(buffer,"REG_ADMIN: %d",&id) != 1){
                                perror("sscanf");
                                continue;
                            }
                            if(id < 1 || id > 15){
                                printf("Please enter right window number\n");
                                continue;
                            }
                            windows[id - 1].admin_fd = client_sockets[i];
                            
                        }
                        else if(!strcmp(buffer,"REG_TV")) tv_fd = client_sockets[i];    
                        
                        int ticket_found = 0;
                        int current_window_index = -1;
                        int next_client_index = -1;
                        if(!strcmp(buffer,"CMD_NEXT")){
                            for(int w = 0; w < WINDOWS_COUNT; ++w) {
                                if(windows[w].admin_fd == client_sockets[i]) {
                                    current_window_index = w;
                                    break;
                                }
                            }
                            if (current_window_index != -1) {
                                
                                char reply_text[50];
                                int best_ticket_index = -1;
                                int best_priority = -1;
                                for(int next_index = 0; next_index < ticket_count; ++next_index){
                                    int ticket_service_index = queue[next_index].service_index;
                                    
                                    if(queue[next_index].status == 0 && windows[current_window_index].allowed_services[ticket_service_index] == 1){
                                        if (queue[next_index].priority > best_priority) {
                                            best_priority = queue[next_index].priority;
                                            best_ticket_index = next_index;
                                        }
                                    }
                                }
                                if(best_ticket_index != -1){
                                    queue[best_ticket_index].status = 1;
                                    queue[best_ticket_index].window_num = windows[current_window_index].window_number;
                                    windows[current_window_index].status = 1;
                                    
                                    sprintf(reply_text, "SERVING: %s", queue[best_ticket_index].ticket_place);
                                    ticket_found = 1;
                                    next_client_index = best_ticket_index;
                                    
                                    if(tv_fd > 0){
                                        char tv_text[50] = "";
                                        sprintf(tv_text, "DISPLAY: Ticket %s -> Window %d", queue[next_client_index].ticket_place, queue[next_client_index].window_num);
                                        write(tv_fd, tv_text, strlen(tv_text) + 1);
                                    }
                                    write(client_sockets[i], reply_text, strlen(reply_text) + 1); 
                                }
                                else if(ticket_found == 0){
                                    char reply_text[20] = "EMPTY";  
                                    write(client_sockets[i],reply_text,strlen(reply_text) + 1);
                                }
                            }
                        }
                        else if(!strcmp(buffer,"CMD_SERVING")){
                            for(int w = 0; w < WINDOWS_COUNT; ++w) {
                                if(windows[w].admin_fd == client_sockets[i]) {
                                    current_window_index = w;
                                    break;
                                }
                            }
                            int curr_ticket_id = -1;
                            if(current_window_index != -1 && tv_fd > 0){
                                for(int t = 0; t < ticket_count;++t){
                                    if(queue[t].status == 1 && queue[t].window_num == windows[current_window_index].window_number){
                                        curr_ticket_id = t;
                                        break;
                                    }
                                }
                                if(curr_ticket_id != -1){
                                    char msg[100] = {0};
                                    sprintf(msg, "DELETE: Ticket %s -> Window %d",queue[curr_ticket_id].ticket_place, queue[curr_ticket_id].window_num);
                                    printf("uxarkum emm: %s\n",msg);
                                    write(tv_fd,msg,strlen(msg) + 1);
                                    queue[curr_ticket_id].status = 2;
                                }
                            }
                        }
                        else if(!strcmp(buffer,"CMD_SERVED")){
                            for(int w = 0; w < WINDOWS_COUNT; ++w) {
                                if(windows[w].admin_fd == client_sockets[i]) {
                                    current_window_index = w;
                                    break;
                                }
                            }
                            if(current_window_index != -1){
                                windows[current_window_index].status = 0;
                                for(int t = 0; t < ticket_count; ++t){
                                    if(queue[t].status == 1 && queue[t].window_num == windows[current_window_index].window_number){
                                        queue[t].status = 3;
                                        break;
                                    }
                                }
                            }
                        }
                        else if (!strncmp(buffer, "CMD", strlen("CMD"))) {
                            if (client_sockets[i] == kiosk_fd) {
                                memset(&queue[ticket_count], 0, sizeof(ticket_example));
                                if(ticket_count == QUEUE_MAX){
                                    const char queue_full_msg[15] = "QUEUE_FULL";
                                    write(client_sockets[i],queue_full_msg,strlen(queue_full_msg));
                                    continue;
                                }
                                int chosen_priority = (int) (buffer[strlen(buffer) - 1] - '0');
                                char new_buffer[1024] = {0};
                                strncpy(new_buffer,buffer,strlen(buffer) - strlen("P-2"));
                                new_ticket(new_buffer,queue[ticket_count].ticket_place,service_queue);
                                queue[ticket_count].status = 0;
                                queue[ticket_count].service_index = 0;
                                queue[ticket_count].priority = chosen_priority;
                                for(int s_index = 0; s_index < AVAILABLE_SERVICE_LIST_LENGTH; ++s_index){
                                    if(!strcmp(new_buffer,service_msgs[s_index])){
                                        queue[ticket_count].service_index = s_index;
                                    }
                                }  
                                time_for_tickets(queue[ticket_count].create_time, queue[ticket_count].create_date);
                                char server_msg[50] = "Ticket: ";
                                strcat(server_msg, queue[ticket_count].ticket_place);
                                write(client_sockets[i], server_msg, strlen(server_msg) + 1);
                                
                                printf("DEBUG: ticket_place = '%s', strlen = %lu\n", queue[ticket_count].ticket_place, strlen(queue[ticket_count].ticket_place));
                                char sql_msg[512] = {0};
                                snprintf(sql_msg, sizeof(sql_msg),
                                    "INSERT INTO bank_tickets (ticket_place, service_index, priority, create_time, create_date) VALUES ('%s', %d, %d, '%s', '%s')",
                                    queue[ticket_count].ticket_place, 
                                    queue[ticket_count].service_index,  
                                    queue[ticket_count].priority, 
                                    queue[ticket_count].create_time, 
                                    queue[ticket_count].create_date);
                                if(mysql_query(conn, sql_msg)){
                                    fprintf(stderr, "Insert error: %s\n", mysql_error(conn));
                                }
                                ++ticket_count;

                            }
                        }        
                    }
                }
            }
        }
        close(fd);
    }