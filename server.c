/* 
    Student: Joao Antunes (2018295351)
    Server Application
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
// Credit to Ricardo Garcia for Hashing Library (Bcrypt)
// Repository link: https://github.com/rg3/libbcrypt
// To make file: gcc server.c -o server -lcurl -lsodium -lcrypto -ljson-c crypt_blowfish/*.o    
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
// API Libraries
#include <json-c/json.h>
#include <curl/curl.h>
#include "isabela_api.c"
// Hashing Library
#include "bcrypt.c"
// Symmetric Encryption
#include "aes_symmetric.c"
// Asymmetric Encryption
#include <sodium.h>

// ======== Symmetric Encryption ========
/* A 256 bit key */
unsigned char key[32];
/* A 128 bit IV */
unsigned char iv[16];
unsigned char ciphertext[128];
/* Buffer for the decrypted text */
unsigned char decryptedtext[128];
int decryptedtext_len, ciphertext_len;

// ======== Define ========
// Clear Screen
#define clear() printf("\033[H\033[J")
// Server PORT
#define SERVER_PORT 9001
#define BUF_SIZE 1024

// ======== Function Declaration ========
// Function declaration
void receive_symmetric_key(int client_fd);
void receive_symmetric_iv(int client_fd);
void process_client(int client_fd);
void check_id(int client_fd);
const char *get_student_information(int client_fd, int option_flag, const char *user_id);
void data(int client_fd, const char *user_id);
void private_data(int client_fd, char data[11][BUF_SIZE], const char *user_id);
void group_data(int client_fd, char data[6][BUF_SIZE], const char *user_id);
void send_data_routine(int client_fd, char *data_cell);
void check_user(int client_fd, const char *user_id);
void login(int client_fd, const char* user_id, char info[2][BUF_SIZE]);
void registration(int client_fd, const char *user_id);
void receive_data(int client_fd, char *data);
void generate_hash(const char *password);
// Error Message
void error(char *msg);

int main()
{
    int fd, client;
    struct sockaddr_in addr, client_addr;
    int client_addr_size;

    // Clear initial screen and tell the server is running
    clear();
    printf("Server on Port %d\nServer running...\n", SERVER_PORT);

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Error Socket");
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        error("Error Binding");
    if (listen(fd, 5) < 0)
        error("Error Listening");
    while(1){
        client_addr_size = sizeof(client_addr);
        client = accept(fd,(struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
        if (client > 0) {
            if (fork() == 0) {
                close(fd);
                // Receives KEY for Symmetric Encryption
                receive_symmetric_key(client);
                // Receives IV for Symmetric Encryption
                receive_symmetric_iv(client);
                printf("Client joined...\n");
                // Returns to normal execution
                check_id(client);
                exit(0);
            }
        close(client);
        }
    }
    return 0; 
}

void receive_symmetric_key(int client_fd)
{    
    char buffer[BUF_SIZE];
    int nread;
    unsigned char recipient_pk[crypto_box_PUBLICKEYBYTES];
    unsigned char recipient_sk[crypto_box_SECRETKEYBYTES];
    // Generation of new Public/Private Key Pair
    crypto_box_keypair(recipient_pk, recipient_sk);
    // Sends to client public key
    write(client_fd, recipient_pk, strlen(recipient_pk));
    // Receives ciphertext length of encrypted message sent by the user
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    int cipher_len;
    cipher_len = atoi(buffer);
    // Receives ciphertext sent by the user
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    unsigned char cipher[cipher_len];
    strcpy(cipher, buffer);
    unsigned char decrypted[32];
    // Decrypts message sent
    if (crypto_box_seal_open(decrypted, cipher, cipher_len,
                            recipient_pk, recipient_sk) != 0) {
        /* message corrupted or not intended for this recipient */
    }
    // Assigns decrypted message value to key
    strcpy(key, decrypted);
}

void receive_symmetric_iv(int client_fd)
{    
    char buffer[BUF_SIZE];
    int nread;
    unsigned char recipient_pk[crypto_box_PUBLICKEYBYTES];
    unsigned char recipient_sk[crypto_box_SECRETKEYBYTES];
    // Generation of new Public/Private Key Pair
    crypto_box_keypair(recipient_pk, recipient_sk);
    // Sends to client public key
    write(client_fd, recipient_pk, strlen(recipient_pk));
    // Receives ciphertext length of encrypted message sent by the user
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    int cipher_len;
    cipher_len = atoi(buffer);
    // Receives ciphertext sent by the user
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    unsigned char cipher[cipher_len];
    strcpy(cipher, buffer);
    unsigned char decrypted[32];
    // Decrypts message sent
    if (crypto_box_seal_open(decrypted, cipher, cipher_len,
                            recipient_pk, recipient_sk) != 0) {
        /* message corrupted or not intended for this recipient */
    }
    // Assigns decrypted message value to iv
    strcpy(iv, decrypted);
}

void check_id(int client_fd){
/* 
    Function that check User's ID
*/
    int nread = 0;
    char buffer[BUF_SIZE];
    char send_id[BUF_SIZE];
    // Reads buffer given by client with User's ID
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    decryptedtext_len = decrypt(buffer, strlen(buffer), key, iv, decryptedtext);
    strcpy(buffer, decryptedtext);
    // Gives User's ID to function 
    // Option Flag = 0 (No Data Retrieved)
    const char *user_id = get_student_information(client_fd, 0, buffer);
    // Checks if User's ID is valid
    // If not valid returns to client an information string
    if(strcmp(user_id, "User not found") == 0)
    {
        strcat(buffer, " not found");
        ciphertext_len = encrypt (buffer, strlen ((char *)buffer), key, iv,
                              ciphertext);
        write(client_fd, ciphertext, ciphertext_len);
        check_id(client_fd);
    }
    // If valid returns to client and information string
    else{
        strcat(buffer, " was found");
        ciphertext_len = encrypt (buffer, strlen ((char *)buffer), key, iv,
                              ciphertext);
        write(client_fd, ciphertext, ciphertext_len);
        // Receives Option from Client (Either Private or Group Data)
        check_user(client_fd, user_id);
    }    
}

const char *get_student_information(int client_fd, int option_flag, const char *user_id){  
    //JSON obect
	struct json_object *jobj_array, *jobj_obj;
	struct json_object *jobj_object_id, *jobj_object_type, *jobj_object_activity, *jobj_object_location, *jobj_object_latlong, *jobj_object_callsduration, 
	*jobj_object_callsmade, *jobj_object_callsmissed, *jobj_object_callsreceived, *jobj_object_department, *jobj_object_smsreceived, *jobj_object_smssent;
	enum json_type type = 0;
	int arraylen = 0;
	int i;
	//Get the student data
	jobj_array = get_student_data();

	//Get array length
	arraylen = json_object_array_length(jobj_array);
    // Variable initialization for group data
    float callsduration = 0, callsmade = 0, callsmissed = 0, callsreceived = 0, smsreceived = 0, smssent = 0; 
	//Example of howto retrieve the data
	for(i = 0; i < arraylen; i++){
		//get the i-th object in jobj_array
		jobj_obj = json_object_array_get_idx(jobj_array, i);
		//get the name attribute in the i-th object
		jobj_object_id = json_object_object_get(jobj_obj, "id");
		jobj_object_type = json_object_object_get(jobj_obj, "type");
		jobj_object_activity = json_object_object_get(jobj_obj, "activity");
		jobj_object_location = json_object_object_get(jobj_obj, "location");
		jobj_object_callsduration = json_object_object_get(jobj_obj, "calls_duration");
		jobj_object_callsmade = json_object_object_get(jobj_obj, "calls_made");
		jobj_object_callsmissed = json_object_object_get(jobj_obj, "calls_missed");
		jobj_object_callsreceived= json_object_object_get(jobj_obj, "calls_received");
		jobj_object_department = json_object_object_get(jobj_obj, "department");
		jobj_object_smsreceived = json_object_object_get(jobj_obj, "sms_received");
		jobj_object_smssent = json_object_object_get(jobj_obj, "sms_sent");
        
        // Assigning values to variables
        char priv_data[11][BUF_SIZE];
        strcpy(priv_data[0], json_object_get_string(jobj_object_id));
        strcpy(priv_data[1], json_object_get_string(jobj_object_type));
        strcpy(priv_data[2], json_object_get_string(jobj_object_activity));
        strcpy(priv_data[3], json_object_get_string(jobj_object_location));
        strcpy(priv_data[4], json_object_get_string(jobj_object_callsduration));
        strcpy(priv_data[5], json_object_get_string(jobj_object_callsmade));
        strcpy(priv_data[6], json_object_get_string(jobj_object_callsmissed));
        strcpy(priv_data[7], json_object_get_string(jobj_object_callsreceived));
        strcpy(priv_data[8], json_object_get_string(jobj_object_department));
        strcpy(priv_data[9], json_object_get_string(jobj_object_smsreceived));
        strcpy(priv_data[10], json_object_get_string(jobj_object_smssent));
        // Assigning values to variables
        const char *id = json_object_get_string(jobj_object_id);
        // Temporary solution (Sends data if User's )
        if(option_flag == 0){
            //if(strcmp(id, user_id) == 0){
            // ciphertext_len = encrypt (id, strlen ((char *)id), key, iv,
            //                 ciphertext);
            // decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv, decryptedtext);
            // strcpy(id, decryptedtext);
            if(strstr(user_id, id) != NULL){
                return id;
            } 
        }
        // Option Flag == 1 (Group Data) Sends private data
        if(option_flag == 1){
            if(strstr(user_id, id) != NULL){
                private_data(client_fd, priv_data, user_id);
            }
        }
        // Option Flag = 2 (Group Data) Sums all the data parcels 
        if(option_flag == 2 || i == arraylen){
            callsduration += atof(json_object_get_string(jobj_object_callsduration));
            callsmade += atof(json_object_get_string(jobj_object_callsmade));
            callsmissed += atof(json_object_get_string(jobj_object_callsmissed));
            callsreceived += atof(json_object_get_string(jobj_object_callsreceived));
            smsreceived += atof(json_object_get_string(jobj_object_smsreceived));
            smssent += atof(json_object_get_string(jobj_object_smssent));
        }  
	}
    // Calculates average of the data that is considered safe to share
    char all_data[6][BUF_SIZE];
    if(option_flag == 2){
        gcvt((callsduration / arraylen), 4, all_data[0]);
        gcvt((callsmade / arraylen), 4, all_data[1]);
        gcvt((callsmissed / arraylen), 4, all_data[2]);
        gcvt((callsreceived / arraylen), 4, all_data[3]);
        gcvt((smsreceived / arraylen), 4, all_data[4]);
        gcvt((smssent / arraylen), 4, all_data[5]);
        group_data(client_fd, all_data, user_id);
    }
    // Temporary Solution
    return "User not found";  
    
}

void data(int client_fd, const char *user_id){
/* 
    Function that receives the chosen option from the client
*/
    int nread = 0, option = -1;
    char buffer[BUF_SIZE] = "";
    // Reads User's Choice (Private or Group Data)
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    decryptedtext_len = decrypt(buffer, strlen(buffer), key, iv, decryptedtext);
    strcpy(buffer, decryptedtext);
    if(strstr(buffer, "1") != NULL){
        // Flag = 1 (Private Data)
        get_student_information(client_fd, 1, user_id);
    }
    if(strstr(buffer, "2") != NULL){
        // Flag = 2 (Group Data)
        get_student_information(client_fd, 2, user_id);
    }
    data(client_fd, user_id);
}

void private_data(int client_fd, char priv_data[11][BUF_SIZE], const char *user_id)
{
/* 
    Function sends private data to the client app
*/
    int i = 0;
    while(i < 11){
        int nread = 0;
        char buffer[BUF_SIZE];
        // Sends encrypted data
        ciphertext_len = encrypt (priv_data[i], strlen ((char *)priv_data[i]), key, iv,
                              ciphertext);
        write(client_fd, ciphertext, ciphertext_len);
        while(nread == 0){
            nread = read(client_fd, buffer, BUF_SIZE-1);
        }
        i++;
    }
    data(client_fd, user_id);
     
}

void group_data(int client_fd, char all_data[6][BUF_SIZE], const char *user_id)
{
    int i = 0;
    while(i < 6){
        int nread = 0;
        char buffer[BUF_SIZE];
        unsigned char ciphertext[128];
        strcpy(buffer, all_data[i]);
        // Sends encrypted data
        ciphertext_len = encrypt (buffer, strlen ((char *)buffer), key, iv,
                              ciphertext);
        write(client_fd, ciphertext, ciphertext_len);
        while(nread == 0){
            nread = read(client_fd, buffer, BUF_SIZE-1);
        } 
        i++;
    }
    data(client_fd, user_id);
}

void check_user(int client_fd, const char *user_id)
{
    char line[100];
    char * token, info[2][BUF_SIZE];
    int nread;
    char buffer[BUF_SIZE];
    // Waits for an acknowledge from the user (syncing porpuses)
    while(strcmp(buffer, "ack") != 0)
    {
        nread = read(client_fd, buffer, BUF_SIZE-1);
        buffer[nread] = '\0';
        fflush(stdout);
    }
    FILE *fp;
    fp = fopen("database.txt", "r");
    // Reads each line of file
    while(fgets(line, sizeof(line), fp)) {
        // Separates each line between username and password
        //                              info[0]      info[1]
        token = strtok(line, " ");
        int i = 0;
        while(token != NULL){
            strcpy(info[i], token); 
            token = strtok(NULL, " ");
            i++;
            
        }    
        // Checks if User ID given by User exists in database
        if(strcmp(info[0], user_id) == 0)
        {   
            // Removes \n char from the string
            info[1][strlen(info[1]) - 1] = 0;
            // Checks if Password exists in database for that user
            // If it doesn't sends user to registration procedure
            if(strlen(info[1]) < 1)
            {
                // If info[1] is too small is an indication either of an error
                // or that just the user is on that database
                // so it sends that user to registratio
                // This shouldn't happen although
                registration(client_fd, user_id);
            }
            // If it does, sends user to login procedure
            else{
                login(client_fd, user_id, info);
            }
            
        }
        // In case of empty database
        else
        {
            registration(client_fd, user_id);
        }
    }
    fclose(fp);
    // Just for safety
    registration(client_fd, user_id);
}

void login(int client_fd, const char* user_id, char info[2][BUF_SIZE])
{
    int nread;
    char buffer[BUF_SIZE];
    // Sends login procedure flag to user
    strcpy(buffer, "login");
    ciphertext_len = encrypt (buffer, strlen ((char *)buffer), key, iv,
                              ciphertext);
    write(client_fd, ciphertext, ciphertext_len);
    // Reads password sent by user
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    decryptedtext_len = decrypt(buffer, strlen(buffer), key, iv, decryptedtext);
    strcpy(buffer, decryptedtext); 
    // Checks if password given for login is the same as the one hashed on the database
    // If it's successful will go to the data routine giving access to the menu
    if( bcrypt_checkpw(buffer, info[1]) == 0 )
    {
        strcpy(buffer, "success");
        ciphertext_len = encrypt (buffer, strlen ((char *)buffer), key, iv,
                              ciphertext);
        write(client_fd, ciphertext, ciphertext_len);
        data(client_fd, user_id);
    }
    // If it's unsuccessful, will return the user to the initial menu
    else
    {
        strcpy(buffer, "failed");
        ciphertext_len = encrypt (buffer, strlen ((char *)buffer), key, iv,
                              ciphertext);
        write(client_fd, ciphertext, ciphertext_len);
        check_id(client_fd);
    }
}

void registration(int client_fd, const char *user_id)
{
    int nread;
    char buffer[BUF_SIZE];
    // Opens file
    FILE *fp;
    fp = fopen("database.txt", "a");
    // Sends to user that registration is needed
    strcpy(buffer, "register");
    ciphertext_len = encrypt (buffer, strlen ((char *)buffer), key, iv,
                              ciphertext);
    write(client_fd, ciphertext, ciphertext_len);
    // Writes the user_id to the database
    strcat(user_id, " ");
    fputs(user_id, fp);
    fclose(fp);
    // Reads password given by user
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    decryptedtext_len = decrypt(buffer, strlen(buffer), key, iv, decryptedtext);
    strcpy(buffer, decryptedtext); 
    // Generates a bcrypt hashed password
    // With salt and 12 rounds
    generate_hash(buffer);
    strcpy(buffer, "success");
    ciphertext_len = encrypt (buffer, strlen ((char *)buffer), key, iv,
                              ciphertext);
    write(client_fd, ciphertext, ciphertext_len);
    check_id(client_fd);
}
// Bcrypt Hashing - Safe way of storing passwords
// Use of salt in order to not be predictable
void generate_hash(const char *password)
{
    FILE *fp;
    fp = fopen("database.txt", "a");
    char salt[BCRYPT_HASHSIZE];
	char hash[BCRYPT_HASHSIZE];
    bcrypt_gensalt(12, salt);
    bcrypt_hashpw(password, salt, hash);
    strcat(hash, "\n");
    fputs(hash, fp);
    fclose(fp);
}

void error(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}