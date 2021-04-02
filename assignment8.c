#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#define PORT_NUMBER 49999

void createServer();
void createClient(char*);

int main(int argc, char const *argv[]){
    // server
    if (argc == 2 && strcmp("server", argv[1]) == 0) {
        createServer();
    }

    // client
    else if (argc == 3 && strcmp("client", argv[1]) == 0) {
        char address[20];
        strcpy(address, argv[2]);
        createClient(address);
    }
    else {
        printf("Error: ./assignment8 client address <OR> ./assignment8 server\n");
        return 0;
    }
    return 0;
}

void createServer() {
    int socketFd, listenFd;
    struct sockaddr_in servAddr;
    int length = sizeof(struct sockaddr_in);
    struct sockaddr_in clientAddr;

    // make a socket
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(1);
    }

    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(1);
    }

    // Bind the socket to a port
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT_NUMBER);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socketFd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(1);
    }

    // Listen and Accept connections
    if (listen(socketFd, 1) < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(1);
    }

    char messageRead[20], buffer[20];
    int connectCounter = 0;
    char temp[2];
    char clientName[100] = "";

    while (1) {
        if ((listenFd = accept(socketFd, (struct sockaddr *) &clientAddr, &length)) < 0) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(1);
        }

        char hostName[120];
        int hostEntry;

        hostEntry = getnameinfo((struct sockaddr*)&clientAddr,
                                sizeof(clientAddr),
                                hostName,
                                sizeof(hostName),
                                NULL,
                                0,
                                NI_NUMERICSERV);

        if (hostEntry != 0) {
            fprintf(stderr, "Error: %s\n", gai_strerror(hostEntry));
        }

        connectCounter += 1;
        time_t myTime;
        char *timeStr;

        // got connection
        if (fork() == 0) {
            close(socketFd);

            // stdout client hostname and counter
            printf("%s %d\n", hostName, connectCounter);

            time(&myTime);
            timeStr = ctime(&myTime);
            timeStr[strlen(timeStr) - 7] = '\n';
	        timeStr[strlen(timeStr) - 6] = '\0';
		
            write(listenFd , timeStr , strlen(timeStr));

            close(listenFd);
            exit(0);
        }
        close(listenFd);
    }
}

void createClient(char* myAddress) {
    char buffer[19] = {0};
    char messageRead[19] = {0};

    int socketfd;
    struct addrinfo hints, *actualdata;
    memset(&hints, 0, sizeof(hints));
    int err;

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    err = getaddrinfo(myAddress, "49999", &hints, &actualdata);
    if (err != 0) {
        fprintf(stderr, "Error: %s\n", gai_strerror(err));
        exit(1);
    }

    socketfd = socket(actualdata -> ai_family, actualdata -> ai_socktype, 0);
    if (socketfd < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(1);
    }

    if (connect(socketfd, actualdata -> ai_addr, actualdata -> ai_addrlen) < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(1);
    }
    
    while(read(socketfd, buffer, 19) > 0) {
        strcpy(messageRead, buffer);
        write(1, messageRead, strlen(messageRead));
    }
}
