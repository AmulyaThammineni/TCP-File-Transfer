#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define SERV_PORT 5576

int main() {
    int listenfd, connfd, clilen;
    ssize_t n;
    FILE *fp;
    char s[80], f[80];
    struct sockaddr_in servaddr, cliaddr;

    WSADATA wsaData;
    int result;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }

    // Create socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == INVALID_SOCKET) {
        printf("Socket creation failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Initialize server address structure
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    // Bind socket
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("Bind failed: %ld\n", WSAGetLastError());
        closesocket(listenfd);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(listenfd, 1) == SOCKET_ERROR) {
        printf("Listen failed: %ld\n", WSAGetLastError());
        closesocket(listenfd);
        WSACleanup();
        return 1;
    }

    // Accept a connection
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
    if (connfd == INVALID_SOCKET) {
        printf("Accept failed: %ld\n", WSAGetLastError());
        closesocket(listenfd);
        WSACleanup();
        return 1;
    }
    printf("\nClient connected\n");

    // Read filename from client
    int recv_len = recv(connfd, f, sizeof(f) - 1, 0);
    if (recv_len <= 0) {
        printf("Failed to receive filename or client disconnected.\n");
        closesocket(connfd);
        closesocket(listenfd);
        WSACleanup();
        return 1;
    }
    f[recv_len] = '\0'; // Null-terminate the filename

    // Open the file
    fp = fopen(f, "r");
    if (fp == NULL) {
        perror("File open failed");
        closesocket(connfd);
        closesocket(listenfd);
        WSACleanup();
        return 1;
    }
    printf("\nName of the file: %s\n", f);

    // Send file content to client
    while (fgets(s, sizeof(s), fp) != NULL) {
        int bytes_sent = send(connfd, s, strlen(s), 0);
        if (bytes_sent == SOCKET_ERROR) {
            printf("Send error: %ld\n", WSAGetLastError());
            break;
        }
    }

    // Close file and sockets
    fclose(fp);
    closesocket(connfd);
    closesocket(listenfd);
    WSACleanup();

    return 0;
}