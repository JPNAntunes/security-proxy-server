/* 
    Student: Joao Antunes (2018295351)
    Client Application
    Connection Client/Server with TCP Sockets
*/
/*
    Types of Encryption and Security Measures used:
    -> Password Hashing with Salt using Bcrypt Algorithm
    -> Asymmetric Encryption to exchange Symmetric Encryption Key and IV
       (Generated two Public/Private Key Pairs for each execution)
    -> Symmetric Encryption of Messages between the Server and the Client
    -> Cryptographically-Secure Pseudo-Random Number Generator to create
       new Key/IV Pair for Symmetric Encryption in each execution
*/
//================================= Warning ======================================
//          This is my take on the situation! 
//          Bugs happening because of encryption block sizes 
//          can be fixed assigning manually key and iv both on server
//          and client. Switching Asymmetric Encryption Library or method
//          would be best to fix these issues, since they seem to be caused by
//          libsodium library, bad implementation of the library is possible
//          but problems seem to be connectoed to the sending of encrypted data
//          using TCP sockets.
//=================================================================================
// Credit to Ricardo Garcia for Hashing Library (Bcrypt)
// Repository link: https://github.com/rg3/libbcrypt
// Other Libraries used: Lsodium, OpenSSL, CSPRNG by Duthomhas
// Repository link for the CSPRNG Library used: https://github.com/Duthomhas/CSPRNG
// To make file: gcc client.c -o client -lsodium -lcrypto
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sodium.h>
#include "aes_symmetric.c"
#include "CSPRNG/csprng.c"

// ======== Symmetric Encryption ========
// /* A 256 bit key */
unsigned char key[32];
// /* A 128 bit IV */
unsigned char iv[16];
// ======== Symmetric Encryption (Less bugged) ========
// unsigned char *key = "12345678901234567890123456789012";
// unsigned char *iv = "1234567890123456";

unsigned char ciphertext[128];
/* Buffer for the decrypted text */
unsigned char decryptedtext[128];
int decryptedtext_len, ciphertext_len;

// ======== Define ========
#define BUF_SIZE 1024
// Clear Screen
#define clear() printf("\033[H\033[J")
// Server IP
#define SERVER_IP "127.0.0.1"
// Server PORT
#define SERVER_PORT 9001

// ======== Function Declaration ========
void send_symmetric_key(int fd);
void send_symmetric_iv(int fd);
void error(char *msg);
void send_user_id(int fd);
void authentication_menu(int fd, char *user_id);
void main_menu(int fd);
void display_data(char data[11][BUF_SIZE], int option_flag);
int option_menu();
void return_to_option_menu(int fd);
void send_data(int fd, char *data);

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
    // Cryptographically-Secure Pseudo-Random Number Generator
    // Creates a new Key and IV every execution
    // =======================================================
    CSPRNG rng = csprng_create();
    for (int n = 0; n < 32; n++)
    {
        char c = ((unsigned)csprng_get_int( rng ) % 95) + 32;
        key[n] = c;
    }
    for (int n = 0; n < 16; n++)
    {
        char c = ((unsigned)csprng_get_int( rng ) % 95) + 32;
        iv[n] = c;
    }
    // =======================================================
    // calls functions that send the symmetric encryption key and IV
    // Using Asymmetric Encryption
    send_symmetric_key(fd);
    send_symmetric_iv(fd);
    // Calls main program after Symmetric Encryption Key Trade
    send_user_id(fd);
    close(fd);
    exit(0);
}

void send_symmetric_key(int fd)
{
    char buffer[BUF_SIZE];
    int nread;  
    unsigned char recipient_pk[crypto_box_PUBLICKEYBYTES];
    // Receives the public key from the server
    nread = read(fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    strcpy(recipient_pk, buffer);
    // Sends Symmetric Encryption Key to the Server
	int message_len = strlen(key) + 1;
    int cipher_len = crypto_box_SEALBYTES + message_len;
    unsigned char cipher[cipher_len];
    char length_of_cipher[100];
    sprintf(length_of_cipher, "%d", cipher_len);
    write(fd, length_of_cipher, strlen(length_of_cipher));
    crypto_box_seal(cipher, key, message_len, recipient_pk);
    write(fd, cipher, strlen(cipher));
}

void send_symmetric_iv(int fd)
{
    char buffer[BUF_SIZE];
    int nread;  
    unsigned char recipient_pk[crypto_box_PUBLICKEYBYTES];
    // Receives new public key from the server
    nread = read(fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    strcpy(recipient_pk, buffer);
    // Sends Symmetric Encryption IV to the Server
	int message_len = strlen(iv) + 1;
    int cipher_len = crypto_box_SEALBYTES + message_len;
    unsigned char cipher[cipher_len];
    char length_of_cipher[100];
    sprintf(length_of_cipher, "%d", cipher_len);
    write(fd, length_of_cipher, strlen(length_of_cipher));
    crypto_box_seal(cipher, iv, message_len, recipient_pk);
    write(fd, cipher, strlen(cipher));
}

void send_user_id(int fd){
    char buffer[BUF_SIZE], user_id[50];
    int nread;
    printf("Welcome to ISABELA\nUser ID: ");
    scanf("%s", user_id);
    // Sends give user_id to server
    ciphertext_len = encrypt (user_id, strlen ((char *)user_id), key, iv,
                                ciphertext);
    write(fd, ciphertext, ciphertext_len);
    // Reads Server Request
    nread = read(fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    decryptedtext_len = decrypt(buffer, strlen(buffer), key, iv, decryptedtext);
    strcpy(buffer, decryptedtext); 
    if(strstr(buffer, "not found") != NULL){
        clear();
        printf("%s\n", buffer);
        send_user_id(fd);
    }
    if(strstr(buffer, "was found") != NULL){
        clear();
        printf("--- %s ---\n", buffer);
        // If user id was found in the API
        // Authentication Menu will be presented
        authentication_menu(fd, user_id);
    }
    send_user_id(fd);
}

void authentication_menu(int fd, char *user_id)
{
    char buffer[BUF_SIZE];
    int nread;
    // ACK for syncing porpuses
    write(fd, "ack", strlen("ack"));
    nread = read(fd, buffer, BUF_SIZE-1);
    // Reads server answer
    buffer[nread] = '\0';
    fflush(stdout); 
    decryptedtext_len = decrypt(buffer, strlen(buffer), key, iv, decryptedtext);
    strcpy(buffer, decryptedtext); 
    // If reads register, goes to registration procedure
    if(strstr(buffer, "register") != NULL)
    {
        char password[BUF_SIZE];
        printf("Registration\nNew Password: ");
        scanf("%s", password);
        // Sends user password to the server
        ciphertext_len = encrypt (password, strlen ((char *)password), key, iv,
                                ciphertext);
        write(fd, ciphertext, ciphertext_len);
        nread = read(fd, buffer, BUF_SIZE-1);
        buffer[nread] = '\0';
        fflush(stdout); 
        decryptedtext_len = decrypt(buffer, strlen(buffer), key, iv, decryptedtext);
        strcpy(buffer, decryptedtext); 
        // Receives server answer
        if(strstr(buffer, "success") != NULL){
            clear();
            printf("Registration Successful\n");
            send_user_id(fd);
        }
        else
        {
            error("Registration error! Terminating");
        }
    }
    // If reads login, goes to login procedure
    if(strstr(buffer, "login") != NULL)
    {
        char password[BUF_SIZE];
        printf("Login In\nPassword: ");
        scanf("%s", password);
        // Sends password to server
        ciphertext_len = encrypt (password, strlen ((char *)password), key, iv,
                                ciphertext);
        write(fd, ciphertext, ciphertext_len);
        // Receives answer from server (either success or failed)
        nread = read(fd, buffer, BUF_SIZE-1);
        buffer[nread] = '\0';
        fflush(stdout);
        decryptedtext_len = decrypt(buffer, strlen(buffer), key, iv, decryptedtext);
        strcpy(buffer, decryptedtext); 
        // Reads server answer 
        if(strstr(buffer, "success") != NULL)
        {
            clear();
            printf("Authentication Succesful\n");
            main_menu(fd); 
        }
        if(strstr(buffer, "failed") != NULL)
        {
            clear();
            printf("Authentication Failed\n");
            send_user_id(fd);
        }
    }
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
        strcpy(buffer, "1");
        ciphertext_len = encrypt (buffer, strlen ((char *)buffer), key, iv,
                              ciphertext);
        write(fd, ciphertext, ciphertext_len);
        // While Cycle to get 11 cells of Private Data
        while(i < 11){
            nread = read(fd, buffer, BUF_SIZE-1);
            buffer[nread] = '\0';
            fflush(stdout);
            decryptedtext_len = decrypt(buffer, strlen(buffer), key, iv, decryptedtext);
            strcpy(buffer, decryptedtext); 
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
        strcpy(buffer, "2");
        ciphertext_len = encrypt (buffer, strlen ((char *)buffer), key, iv,
                              ciphertext);
        write(fd, ciphertext, ciphertext_len);
        // While Cycle to get 11 cells of Private Data
        while(i < 6){
            char buffer[BUF_SIZE];
            int nread;
            nread = read(fd, buffer, BUF_SIZE-1);
            buffer[nread] = '\0';
            fflush(stdout);
            decryptedtext_len = decrypt(buffer, strlen(buffer), key, iv, decryptedtext);
            strcpy(buffer, decryptedtext); 
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