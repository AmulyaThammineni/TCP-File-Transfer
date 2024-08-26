#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 5576

// Global Variables
HWND hFileNameInput, hFileContentDisplay;
char fileName[256];

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void RequestFileContent(HWND hwnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "ClientApp";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "File Transfer Client", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hButton;
    switch (uMsg) {
        case WM_CREATE: {
            hFileNameInput = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,
                                          10, 10, 300, 25, hwnd, NULL, NULL, NULL);
            CreateWindow("STATIC", "File Name:", WS_CHILD | WS_VISIBLE,
                         10, 40, 80, 20, hwnd, NULL, NULL, NULL);
            hButton = CreateWindow("BUTTON", "Request File", WS_CHILD | WS_VISIBLE,
                                   320, 10, 150, 30, hwnd, (HMENU)1, NULL, NULL);
            hFileContentDisplay = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                                               10, 70, 460, 280, hwnd, NULL, NULL, NULL);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {  // Button clicked
                RequestFileContent(hwnd);
            }
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        default: {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    return 0;
}

void RequestFileContent(HWND hwnd) {
    // Retrieve file name from input
    GetWindowText(hFileNameInput, fileName, sizeof(fileName));

    if (strlen(fileName) == 0) {
        MessageBox(hwnd, "Please enter a file name.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Create socket and connect to the server
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server;
    char buffer[1024];
    int recvSize;
    SOCKET clientSocket = INVALID_SOCKET;

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET) {
        MessageBox(hwnd, "Socket creation failed.", "Error", MB_OK | MB_ICONERROR);
        WSACleanup();
        return;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(SERVER_PORT);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        MessageBox(hwnd, "Connection failed.", "Error", MB_OK | MB_ICONERROR);
        closesocket(sock);
        WSACleanup();
        return;
    }

    // Send the file name to the server
    send(sock, fileName, strlen(fileName), 0);

    // Receive file content from server
    memset(buffer, 0, sizeof(buffer));
    recvSize = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (recvSize > 0) {
        buffer[recvSize] = '\0';
        SetWindowText(hFileContentDisplay, buffer);
    } else {
        MessageBox(hwnd, "Failed to receive file content.", "Error", MB_OK | MB_ICONERROR);
    }

    // Cleanup
    closesocket(sock);
    WSACleanup();
}