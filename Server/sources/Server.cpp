#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "Server.h"
#include "SocketManager.h"
#include "Constants.h"
#include "ClientInfo.h"

#include <fstream>
#include <sstream>

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
    
    for (auto& thread : m_clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    closesocket(m_listeningSocket);
    WSACleanup();
}

/**
 * std::string to char[]
 * @param source        Исходная строка для копирования.
 * @param destination   Место назначения для копии.
 * @param destSize      Размер буфера назначения.
 */
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
void Server::handleClient(const SOCKET clientSocket, FClientInfo& clientInfo) {
    while (m_serverRunning)
    {
        std::string command = SocketManager::receiveString(clientSocket);

        if (command.empty())
        {
            auto itClient = std::remove_if(m_clients.begin(), m_clients.end(),
                [clientSocket](const FClientInfo& client)
                {
                    return client.sock == clientSocket;
                });
            if (itClient != m_clients.end())
            {
                std::cout << "Client disconnected:\n" << *itClient;
                m_clients.erase(itClient, m_clients.end());
            }

            break; 
        }
        
        if (command == Centaurus::cmd::connect)
        {
            std::string receivedTime = SocketManager::receiveString(clientSocket);
            std::string receivedUser = SocketManager::receiveString(clientSocket);
            std::string receivedName = SocketManager::receiveString(clientSocket);
            std::string receivedIPv4 = SocketManager::receiveString(clientSocket);

            auto itClient = std::find_if(m_clients.begin(), m_clients.end(),
                [clientSocket](const FClientInfo& client)
                {
                    return client.sock == clientSocket;
                });
            if (itClient != m_clients.end())
            {
                CopyReceivedString(receivedTime, itClient->time, sizeof(clientInfo.time));
                CopyReceivedString(receivedUser, itClient->user, sizeof(clientInfo.user));
                CopyReceivedString(receivedName, itClient->name, sizeof(clientInfo.name));
                CopyReceivedString(receivedIPv4, itClient->ipv4, sizeof(clientInfo.ipv4));
            }
            
            std::cout << "Client connected:\n" << *itClient;
        }
        else if (command == Centaurus::cmd::screenshot)
        {
            std::cout << "Requesting screenshot..." << std::endl;

            int dataSize;
            int sizeReceived = recv(clientSocket, reinterpret_cast<char*>(&dataSize), sizeof(dataSize), 0);
            if (sizeReceived == SOCKET_ERROR)
            {
                std::cerr << "Failed to receive size of screenshot, error: " << WSAGetLastError() << std::endl;
                break;
            }

            std::vector<char> screenshotData = SocketManager::receiveData(clientSocket, dataSize);
            if (screenshotData.empty()) return;
            
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
        FClientInfo& clientInfo = m_clients.back();
        clientInfo.sock = clientSocket;
        m_clientThreads.emplace_back(&Server::handleClient, this, clientSocket, std::ref(clientInfo));
    }
}

/**
 * Реализация команд сервера.
 */
void Server::processCommands() {
    std::string command;
    while (m_serverRunning)
    {
        std::getline(std::cin, command);

        if (command.substr(0, Centaurus::cmd::screenshot.length()) == Centaurus::cmd::screenshot)
        {
            std::istringstream iss(command);
            std::string cmd;
            int clientId;

            iss >> cmd;
            iss >> clientId;

            // Поиск клиента по ID
            auto it = std::find_if(m_clients.begin(), m_clients.end(), [clientId](const FClientInfo& client) {
                return client.id == clientId;
            });

            if (it != m_clients.end()) {
                std::cout << "Requesting screenshot from client ID: " << clientId << std::endl;
                send(it->sock, Centaurus::cmd::screenshot.c_str(), Centaurus::cmd::screenshot.size() + 1, 0);
            } else {
                std::cerr << "Client with ID " << clientId << " not found." << std::endl;
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