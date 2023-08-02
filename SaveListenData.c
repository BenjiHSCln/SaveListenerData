/* @author Benjamin Cléon                                                    */
/* This program is conceived to save log whose are received on specific port */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define MAX 1024

// Global variable for signal handler
FILE* file;
int sock;

void flushBuffer(char*, int);
void sighandler(int);
void closeFileAndSocket();

int main(int argc, char** argv)
{
    signal(SIGINT, sighandler);

    // For starting this program, user should put at least two parameter which are the port and the file path+name
    // With the following format : ./SaveListenData -port <port> -path <file path+name>
    if(argc < 5)
    {
        printf("Usage : ./SaveListenData -port <port> -path <file path+name>\n");
        exit(1);
    }

    // Check where is write -port option
    int place_port = 1;
    while(place_port < argc && strcmp(argv[place_port], "-port") != 0)place_port++;

    // Check where is write -path option
    int place_path = 1;
    while(place_path < argc && strcmp(argv[place_path], "-path") != 0)place_path++;

    // Check if -port and -path options are present
    if(place_port == argc || place_path == argc)
    {
        printf("Usage : ./SaveListenData -port <port> -path <file path+name>\n");
        exit(1);
    }

    // Save port and path value according to their place
    char* port = argv[place_port + 1];
    char* path = argv[place_path + 1];

    // Create socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Bind socket to port
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
    server.sin_addr.s_addr = INADDR_ANY;
    if(bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        perror("Bind scoket to port failed");
        exit(1);
    }

    // The file can already exist or not
    file = fopen(path, "a");
    if(file == NULL)
    {
        perror("File opening failed");
        exit(1);
    }

    // Receive data
    char buffer[MAX];
    int size = sizeof(server);
    while(1)
    {

        flushBuffer(buffer, MAX);
        recvfrom(sock, buffer, MAX, 0, (struct sockaddr*)&server, &size);
        fprintf(file, "%s", buffer);
        fflush(file);
    }
}

// Function to flush buffer for not writing to much data at the same time
// And for not having old data in the buffer
void flushBuffer(char* buffer, int size)
{
    int i;
    for(i = 0; i < size; i++)
    {
        buffer[i] = '\0';
    }
}

// Function to close file and socket when program is stopped
void closeFileAndSocket()
{
    fclose(file);
    close(sock);
}

// Function to handle signal
void sighandler(int signum)
{
    closeFileAndSocket();
    exit(1);
}
