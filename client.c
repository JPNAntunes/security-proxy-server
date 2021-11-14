// Sockets TCP
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUF_SIZE 1024
// Clear Screen
#define clear() printf("\033[H\033[J")
// Server IP
#define SERVER_IP "127.0.0.1"
// Server PORT
#define SERVER_PORT 9001

void error(char *msg);

int main(int argc, char *argv[]){
    char endServer[100], buffer[BUF_SIZE], user_id[50];
    int fd, nread;
    struct sockaddr_in addr;
    struct hostent *hostPtr;

    strcpy(endServer, SERVER_IP);
    if ((hostPtr = gethostbyname(endServer)) == 0)
        error("Couldn't resolve address");

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short) SERVER_PORT);

    if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
        error("socket");
    if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
        error("Connect");

    // Giving User's ID
    while(1){
        printf("Welcome to ISABELA\nUser ID: ");
        scanf("%s", user_id);
        // Sends give user_id to server
        write(fd, user_id, strlen(user_id));
        // Reads Server Request
        nread = read(fd, buffer, BUF_SIZE-1);
        buffer[nread] = '\0';
        fflush(stdout); 
        clear();
        // If User is not found cycle continues
        if(strstr(buffer, "not found") != NULL){
            printf("%s\n", buffer);
            continue;
        }
        // If user is found breaks cycle
        else{
            printf("%s\n", buffer);
            break;
        }     
    }
    printf("Continue");
    close(fd);
    exit(0);
}

void error(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}