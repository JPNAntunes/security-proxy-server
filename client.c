/* 
    Student: Joao Antunes (2018295351)
    Client Application
    USER: SRC_USER_1
*/
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
#define SERVER_PORT 9000
//? User ID. Should this app be assigned a USER ID to simulate phone application?
#define USER_ID "SRC_USER_1"
// Function Declaration
void error(char *msg);
void send_user_id(int fd);
void main_menu(int fd);
int option_menu();
void return_to_option_menu(int fd);

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
        
        send_user_id(fd);
    // Giving User's ID
    // while(strstr(buffer, "not found") != NULL){
    //     clear();
    //     printf("%s\n", buffer);
    //     printf("Welcome to ISABELA\nUser ID: ");
    //     scanf("%s", user_id);
    //     // Sends give user_id to server
    //     write(fd, user_id, strlen(user_id));
    //     // Reads Server Request
    //     nread = read(fd, buffer, BUF_SIZE-1);
    //     buffer[nread] = '\0';
    //     fflush(stdout); 
    //     clear();
    //     printf("%s", buffer);
        // If User is not found cycle continues
        // if(strstr(buffer, "not found") != NULL){
        //     clear();
        //     printf("%s\n", buffer);
        // }
        // If user is found breaks cycle
        // else{
        //     printf("---- %s ----\n", buffer);
        //     break;    
        // }
    //}
    // Calling main_menu() function
    //main_menu(fd);
    close(fd);
    exit(0);
}

void send_user_id(int fd){
    char buffer[BUF_SIZE], user_id[50];
    int nread;
    printf("Welcome to ISABELA\nUser ID: ");
    scanf("%s", user_id);
    // Sends give user_id to server
    write(fd, user_id, strlen(user_id));
    // Reads Server Request
    nread = read(fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout); 
    if(strstr(buffer, "not found") != NULL){
        clear();
        printf("%s\n", buffer);
        send_user_id(fd);
    }
    if(strstr(buffer, "was found") != NULL){
        clear();
        printf("--- %s ---\n", buffer);
        main_menu(fd);
    }
    send_user_id(fd);
}

void main_menu(int fd){
    int option = 0, nread;
    char buffer[BUF_SIZE];
    option = option_menu();
    if(option == 1){
        clear();
        printf("Private Data:");
        write(fd, "private_data", strlen("private_data"));
        nread = read(fd, buffer, BUF_SIZE-1);
        buffer[nread] = '\0';
        fflush(stdout);
        printf("%s", buffer);
        return_to_option_menu(fd);
    }
    if(option == 2){
        clear();
        printf("Group Data:");
        return_to_option_menu(fd);
    }
    if(option == 3){
        clear();
        close(fd);
        printf("Application Ended");
        exit(0);
    }
    option_menu();
}

int option_menu(){
    int option;
    while(option != 1 && option != 2 && option != 3){
        printf("Menu\n1 - Private Data\n2 - Group Data\n3 - Exit Application\nOption - ");
        scanf("%d", &option);
        clear();
    }
    return option;
}

void return_to_option_menu(int fd){
    int option = 0;
    while(option != 1 && option != 2){
        printf("\nReturn Back (1) - Exit (2)\nOption - ");
        scanf("%d", &option);
        clear();
    }
    if(option == 1){
        main_menu(fd);
    }
    if(option == 2){
        printf("Application Ended");
        close(fd);
        exit(0);
    }
    
}

void error(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}