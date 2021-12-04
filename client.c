/* 
    Student: Joao Antunes (2018295351)
    Client Application
    Connection Client/Server with TCP Sockets
*/
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
//? User ID. Should this app be assigned a USER ID to simulate phone application?
#define USER_ID "SRC_USER_1"
// Function Declaration
void error(char *msg);
void send_user_id(int fd);
void main_menu(int fd);
void display_data(char data[11][BUF_SIZE], int option_flag);
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
    // Clear initial screen
    clear();
    send_user_id(fd);
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
    int option = 0, nread, i = 0;
    char buffer[BUF_SIZE], data[11][BUF_SIZE];
    // Calls function option_menu() in order for the user to choose option
    option = option_menu();
    if(option == 1){
        clear();
        printf("Private Data:\n");
        // Number 1 for private data
        write(fd, "1", strlen("1"));
        // While Cycle to get 11 cells of Private Data
        while(i < 11){
            nread = read(fd, buffer, BUF_SIZE-1);
            buffer[nread] = '\0';
            fflush(stdout);
            strcpy(data[i], buffer);
            // Sends and Acknowledgment so it can send single char array instead of whole array
            write(fd, "ACK", strlen("ACK"));
            i++;
        }
        // Calls display_data() function in order to print out the data received
        display_data(data, 1);
        // Function to ask for the return to the option menu (or exit)
        return_to_option_menu(fd);
    }
    if(option == 2){
        clear();
        printf("Group Data:\n");
        // Number two for group data
        write(fd, "2", strlen("2"));
        // While Cycle to get 11 cells of Private Data
        while(i < 6){
            nread = read(fd, buffer, BUF_SIZE-1);
            buffer[nread] = '\0';
            fflush(stdout);
            strcpy(data[i], buffer);
            // Sends and Acknowledgment so it can send single char array instead of whole array
            write(fd, "ACK", strlen("ACK"));
            i++;
        }
        //Calls display_data() function in order to print out the data received
        display_data(data, 2);
        //Function to ask for the return to the menu (or exit)
        return_to_option_menu(fd);
    }
    if(option == 3){
        // Exit application
        clear();
        close(fd);
        printf("Application Ended");
        exit(0);
    }
    option_menu();
}

void display_data(char data[11][BUF_SIZE], int option_flag){
/* 
    Function to display data received
*/
    // Printing Private Data Menu
    if(option_flag == 1){
        printf("===========\nID - %s\n", data[0]);
        printf("Type - %s\n", data[1]);
        printf("Activity - %s\n", data[2]);
        printf("Location - %s\n", data[3]);
        printf("Calls Duration - %s\n", data[4]);
        printf("Calls Made - %s\n", data[5]);
        printf("Calls Missed - %s\n", data[6]);
        printf("Calls Received - %s\n", data[7]);
        printf("Department - %s\n", data[8]);
        printf("SMS Received - %s\n", data[9]);
        printf("SMS Sent - %s\n===========", data[10]);
    }
    // Printing Group Data Menu
    if(option_flag == 2){
        printf("===========\nCalls Duration - %s\n", data[0]);
        printf("Calls Made - %s\n", data[1]);
        printf("Calls Missed - %s\n", data[2]);
        printf("Calls Received - %s\n", data[3]);
        printf("SMS Received - %s\n", data[4]);
        printf("SMS Sent - %s\n===========", data[5]);
    }
    
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
        // Go back to Menu
        main_menu(fd);
    }
    if(option == 2){
        // Exit Application
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