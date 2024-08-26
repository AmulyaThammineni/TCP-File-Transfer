#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define SERV_PORT 5576

int main(int argc, char **argv) {
    int sockfd;
    ssize_t n;
    char filename[80], recvline[80];
    struct sockaddr_in servaddr;

    WSADATA wsaData;
    int result;

    // Check if the IP address argument is provided
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Server IP>\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Socket creation failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Initialize server address structure
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        printf("Invalid address or address not supported\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("Connection failed: %ld\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Get filename from user
    printf("Enter the file name: ");
    scanf("%s", filename);

    // Send filename to server
    send(sockfd, filename, sizeof(filename), 0);

    // Read and print data from server
    printf("\nData from server:\n");
    while ((n = recv(sockfd, recvline, sizeof(recvline) - 1, 0)) > 0) {
        recvline[n] = '\0';  // Null-terminate the string
        fputs(recvline, stdout);
    }

    if (n < 0) {
        printf("Read error: %ld\n", WSAGetLastError());
    }

    // Close the socket
    closesocket(sockfd);
    WSACleanup();

    return 0;
}