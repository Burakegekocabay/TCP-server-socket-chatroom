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
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET; // for ipv4
    address->sin_port = htons(2000); //PORT 2000
    address->sin_addr.s_addr = INADDR_ANY;
    return address;
}

struct AcceptedSocket 
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

struct AcceptedSocket acceptedSockets[10];

int acceptedSocketsCount = 0;

struct AcceptedSocket * acceptConn(int serverSocket) 
{
    struct sockaddr_in userAddress;
    int userAddressSize = sizeof(struct sockaddr_in);
    int userSocketFD = accept(serverSocket, (struct sockaddr*)&userAddress, &userAddressSize);

    struct AcceptedSocket* acceptedSocket = malloc(sizeof(struct AcceptedSocket));
    acceptedSocket->address = userAddress;
    acceptedSocket->acceptedSocketFD = userSocketFD;
    acceptedSocket->acceptedSuccessfully = userSocketFD > 0;

    if(!acceptedSocket->acceptedSuccessfully)
        acceptedSocket->error = userSocketFD;

    return acceptedSocket;
}

void receiveAndPrintIncomingData(int socketFD) 
{
    char buffer[1024];

    while(true) 
    {
        ssize_t amountReceived = recv(socketFD, buffer, 1024, 0);

        if(amountReceived > 0) 
        {
            buffer[amountReceived] = 0;
            printf("%s\n", buffer);

            //send messages to all users
            for(int i = 0; i < acceptedSocketsCount; i++) 
                if(acceptedSockets[i].acceptedSocketFD != socketFD) 
                    send(acceptedSockets[i].acceptedSocketFD, buffer, strlen(buffer), 0);

        }

        if(amountReceived == 0)
            break;
    }

    close(socketFD);
}

void startConn(int serverSocket) 
{
    while(true) 
    {
        struct AcceptedSocket* userSocket  = acceptConn(serverSocket);
        acceptedSockets[acceptedSocketsCount++] = *userSocket;

        pthread_t id;
        pthread_create(&id, NULL, (void *(*)(void *))receiveAndPrintIncomingData, (void*)(long)userSocket->acceptedSocketFD);
    }
}


int main() 
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in *serverAddress = createIPv4();

    int result = bind(serverSocket, (struct sockaddr*)serverAddress, sizeof(*serverAddress));
    if(result == 0)
        printf("Connection Succesfull\n");

    int listenResult = listen(serverSocket, 10);

    startConn(serverSocket);

    shutdown(serverSocket, SHUT_RDWR);

    return 0;
}
