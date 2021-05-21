#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

int fd[17];

struct Pipe {
    int fd_send;
    int fd_recv;
    int fd_index;
    int fd_name;
    int fd_code;
};

void *handle_chat1(void *data) {
    struct Pipe *pipe = (struct Pipe *)data;
    char buffer[1024] = "Message";
    buffer[7]='0'+pipe->fd_index;
    buffer[8]=':';
    ssize_t len;
    while ((len = recv(pipe->fd_send, buffer + 9, 1000, 0)) > 0) {
        send(pipe->fd_recv, buffer, len + 9, 0);
    }
    return NULL;
}


void *handle_chat2(void *data) {
    struct Pipe *pipe = (struct Pipe *)data;
    char buffer[1024] = "Message_server:";
    int j;
    ssize_t len;
    while ((len = recv(fd[0], buffer + 15, 1000, 0)) > 0){
        for (j=0;j<16;j++){
            send(fd[j+1],buffer,len+15,0);
        }
    }
    return NULL;
}

int check_name(struct Pipe *pipe){
    char name[10];
    char Name[10];
    int name_index;
    int flag = 0; 
    int i;
    memset(name,0,sizeof(name));
    memset(Name,0,sizeof(Name));
    recv(pipe->fd_send, name, 10, 0);
    for (i = 0;i < 10;i ++){
        if (name[i] == '\n')
            name[i]=0;
    }
    FILE *stream = fopen("name_code.txt","r");
    for (name_index = 0;name_index < 16;name_index ++){
        fscanf(stream,"%s",Name);
        if (strcmp(name,Name) == 0){
            flag = 1;
            break;
        }
        fscanf(stream,"%s",Name);
        memset(Name,0,sizeof(Name));
    }
    fclose(stream);
    return flag;
}

int check_code(struct Pipe *pipe){
    char code[10];
    char Code[10];
    int code_index;
    int flag = 0;
    int i;
    memset(code,0,sizeof(code));
    memset(Code,0,sizeof(Code));
    recv(pipe->fd_send, code, 10, 0);
    for (i = 0;i < 10;i ++){
        if (code[i] == '\n')
            code[i]=0;
    }
    FILE *stream=fopen("name_code.txt","r");
    for (code_index = 0;code_index < 16;code_index ++){
        fscanf(stream,"%s",Code);
        memset(Code,0,sizeof(Code));
        fscanf(stream,"%s",Code);
        if (strcmp(code,Code) == 0){
            flag = 1;
            break;
        }
    }
    fclose(stream);
    return flag;
}


int main(int argc, char **argv) {
    int i=1,j;
    int port = atoi(argv[1]);
    int Fd;
    ssize_t len;
    if ((Fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket");
        return 1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    socklen_t addr_len = sizeof(addr);
    if (bind(Fd, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind");
        return 1;
    }
    if (listen(Fd, 17)) {
        perror("listen");
        return 1;
    }
    fd[0] = accept(Fd, NULL, NULL);
    if (fd[0] == -1){
        perror("accept");
        return 1;
    }
    pthread_t thread[17];
    pthread_t thread1[17];
    struct Pipe pipe[17];
    struct Pipe pipe1[17];
    while (i <= 17){
        fd[i] = accept(Fd, NULL, NULL);
        if (fd[i] == -1){
            perror("accept");
            return 1;
        }
        pipe[i].fd_send = fd[i];
        pipe[i].fd_recv = fd[0];
        pipe1[i].fd_index = i;
        pipe[i].fd_index = i;
        send(fd[i],"please input your username:\n",28,0);
        if (check_name(&pipe[i]) == 0){
            close(fd[i]);
            continue;
        }
        send(fd[i],"please input your password:\n",28,0);
        if (check_code(&pipe[i]) == 0){
            close(fd[i]);
            continue;
        }
        send(fd[i],"succeed in signing in\n", 22, 0);
        pthread_create(&thread[i], NULL, handle_chat2, (void *)&pipe1[i]);
        pthread_create(&thread1[i], NULL, handle_chat1, (void *)&pipe[i]);
        i++;
        }
    return 0;
}
