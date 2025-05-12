#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

struct sockaddr_in* createIPv4()
{
    struct sockaddr_in * address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET; //We use this for IPV4
    address->sin_port = htons(2000);
    inet_pton(AF_INET, "127.0.0.1", &address->sin_addr.s_addr); //Localhost port 2000 
    return address;
}


void chatBOX(int userSocket) 
{
    char *username = NULL;
    size_t usernameSize = 0;
    printf("Please enter your username: ");
    ssize_t usernameCount = getline(&username, &usernameSize, stdin);
    username[usernameCount - 1] = 0;

    char *message = NULL;
    size_t messageSize = 0;
    printf("Type message (type 'exit' to quit)...\n");

    char messageBuffer[1024];

    while(true) 
    {
        ssize_t charCount = getline(&message, &messageSize, stdin);
        message[charCount - 1] = 0;

        sprintf(messageBuffer, "%s:%s", username, message);

        if(charCount > 0) 
        {
            if(strcmp(message, "exit") == 0)
                break;
            send(userSocket, messageBuffer, strlen(messageBuffer), 0);
        }
    }
}

void *listenAndPrint(void *arg) 
{  
    int userSocket = (int)(long)arg; 
    char messageBuffer[1024];

    while(true) 
    {
        ssize_t amountReceived = recv(userSocket, messageBuffer, 1024, 0);

        if(amountReceived > 0) 
        {
            messageBuffer[amountReceived] = 0;
            printf("%s\n", messageBuffer);
        }

        if(amountReceived == 0)
            break;
    }

    close(userSocket);
    return NULL;
}


int main() 
{
    int userSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in *address = createIPv4();

    int result = connect(userSocket, (struct sockaddr*)address, sizeof(*address));

    if(result == 0)
        printf("Connection was successful\n");

    pthread_t id;
    pthread_create(&id, NULL, listenAndPrint, (void*)(long)userSocket);  //Multi - Threading

    chatBOX(userSocket);
    close(userSocket);

    return 0;
}