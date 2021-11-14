//! Remove comments
//! Get Private Data to Client App

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

// Server running on Port 8000
#define SERVER_Port 9000
#define BUF_SIZE 1024

void process_client(int client_fd);
void found_or_not(int client_fd);
const char *get_student_information(int client_fd, int option_flag, char *user_id);
void send_data(int client_fd, char user_id[]);
void send_private_data(int client_fd, char user_id[], const char *id, const char *type, const char *activity, const char *location, const char *calls_duration, const char *calls_made, const char *calls_missed, const char *calls_received, const char *department, const char *sms_received, const char *sms_sent);
// Error Message
void error(char *msg);

int main()
{
    int fd, client;
    struct sockaddr_in addr, client_addr;
    int client_addr_size;

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_Port);

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
                process_client(client);
                exit(0);
            }
        close(client);
        }
    }
    return 0;  
}

void process_client(int client_fd){
    int nread = 0;
    char buffer[BUF_SIZE];
    found_or_not(client_fd);
    // // Reads buffer given by client with User's ID
    // nread = read(client_fd, buffer, BUF_SIZE-1);
    // buffer[nread] = '\0';
    // fflush(stdout);
    // // Gives User's ID to function 
    // const char *user_id = get_student_information(buffer);
    // // Checks if User's ID is valid
    // // If not valid returns to client an information string
    // if(strcmp(user_id, "User not found") == 0){
    //     write(client_fd, strcat(buffer, " not found"), strlen(strcat(buffer, " not found")));
    // }
    // // If valid returns to client and information string
    // else{
    //     write(client_fd, strcat(buffer, " found"), strlen(strcat(buffer, " found")));
        
    // }
    close(client_fd);
}

void found_or_not(int client_fd){
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
        found_or_not(client_fd);
    }
    // If valid returns to client and information string
    else{
        write(client_fd, strcat(buffer, " was found"), strlen(strcat(buffer, " was found")));
        // Receives Option from Client (Either Private or Group Data)
        send_data(client_fd, user_id);
    }    
}

const char *get_student_information(int client_fd, int option_flag, char *user_id){
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

	//Example of howto retrieve the data
	for (i = 0; i < arraylen; i++){
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
        const char *id = json_object_get_string(jobj_object_id);
        const char *type = json_object_get_string(jobj_object_type);
        const char *activity = json_object_get_string(jobj_object_activity);
        const char *location = json_object_get_string(jobj_object_location);
        const char *calls_duration = json_object_get_string(jobj_object_callsduration);
        const char *calls_made = json_object_get_string(jobj_object_callsmade);
        const char *calls_missed = json_object_get_string(jobj_object_callsmissed);
        const char *calls_received = json_object_get_string(jobj_object_callsreceived);
        const char *department = json_object_get_string(jobj_object_department);
        const char *sms_received = json_object_get_string(jobj_object_smsreceived);
        const char *sms_sent = json_object_get_string(jobj_object_smssent);
        // Temporary solution (Sends data if User's )
        if(strcmp(id, user_id) == 0){
            if(option_flag == 0){
                return id;
            }
            if(option_flag == 1){
                send_private_data(client_fd, user_id, id, type, activity, location, calls_duration, calls_made, calls_missed, calls_received, department, sms_received, sms_sent);
            }   
        }
	}
    // Temporary Solution
    return "User not found";
}

void send_data(int client_fd, char user_id[]){
    int nread = 0;
    char buffer[BUF_SIZE];
    // Reads User's Choice (Private or Group Data)
    nread = read(client_fd, buffer, BUF_SIZE-1);
    buffer[nread] = '\0';
    fflush(stdout);
    if(strcmp(buffer, "private_data") == 0){
        // Flag = 1 (Private Data)
        get_student_information(client_fd, 1, user_id);
    }
    send_data(client_fd, user_id);
}

void send_private_data(int client_fd, char user_id[], const char *id, const char *type, const char *activity, const char *location, const char *calls_duration, const char *calls_made, const char *calls_missed, const char *calls_received, const char *department, const char *sms_received, const char *sms_sent){
    write(client_fd, strcat(type, location), strlen(strcat(type, location)));
    send_data(client_fd, user_id);
}

void error(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}