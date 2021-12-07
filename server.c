/* 
    Student: Joao Antunes (2018295351)
    Server Application
    Connection Client/Server with TCP Sockets
*/
// To make file: gcc server.c -o server -lcurl -ljson-c crypt_blowfish/*.o    
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
#include "bcrypt.c"

// Clear Screen
#define clear() printf("\033[H\033[J")
// Server running on Port 8000
#define SERVER_PORT 9000
#define BUF_SIZE 1024

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
                check_id(client);
                exit(0);
            }
        close(client);
        }
    }
    return 0;  
}

void check_id(int client_fd){
/* 
    Function that check User's ID
*/
    int nread = 0;
    char buffer[BUF_SIZE];
    // Reads buffer given by client with User's ID
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    // Gives User's ID to function 
    // Option Flag = 0 (No Data Retrieved)
    const char *user_id = get_student_information(client_fd, 0, buffer);
    // Checks if User's ID is valid
    // If not valid returns to client an information string
    if(strcmp(user_id, "User not found") == 0){
        write(client_fd, strcat(buffer, " not found"), strlen(strcat(buffer, " not found")));
        check_id(client_fd);
    }
    // If valid returns to client and information string
    else{
        write(client_fd, strcat(buffer, " was found"), strlen(strcat(buffer, " was found")));
        // Receives Option from Client (Either Private or Group Data)
        check_user(client_fd, user_id);
        //data(client_fd, user_id);
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
            if(strcmp(id, user_id) == 0){
                return id;
            } 
        }
        // Option Flag == 1 (Group Data) Sends private data
        if(option_flag == 1){
            if(strcmp(id, user_id) == 0){
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
    option = atoi(buffer);
    if(option == 1){
        // Flag = 1 (Private Data)
        get_student_information(client_fd, 1, user_id);
    }
    if(option == 2){
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
        write(client_fd, priv_data[i], strlen(priv_data[i]));
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
        write(client_fd, all_data[i], strlen(all_data[i]));
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
            // Removinf \n char from the string
            //! if((info[1][strlen(info[1]) - 1]) == "\n"){
            info[1][strlen(info[1]) - 1] = 0;
            //! }
            // Checks if Password exists in database for that user
            // If it doesn't sends user to registration procedure
            if(strlen(info[1]) < 1)
            {
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
    registration(client_fd, user_id);
}

void login(int client_fd, const char* user_id, char info[2][BUF_SIZE])
{
    int nread;
    char buffer[BUF_SIZE];
    // Sends login procedure flag to user
    write(client_fd, "login", strlen("login"));
    // Reads password sent by user
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    if(strcmp(info[1], buffer) == 0)
    {
        write(client_fd, "success", strlen("success"));
        data(client_fd, user_id);
    }
    else
    {
        write(client_fd, "failed", strlen("failed"));
        check_id(client_fd);
    }
}

void registration(int client_fd, const char *user_id)
{
    int nread;
    char buffer[BUF_SIZE];

    FILE *fp;
    fp = fopen("database.txt", "a");
    // Sends to user that registration is needed
    write(client_fd, "register", strlen("register"));
    strcat(user_id, " ");
    fputs(user_id, fp);
    fclose(fp);

    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    generate_hash(buffer);
    write(client_fd, "success", strlen("success"));
    check_id(client_fd);
}

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