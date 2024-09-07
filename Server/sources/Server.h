#pragma once

#include <winsock2.h>
#include <vector>
#include <thread>
#include <atomic>
#include <string>

#include "ClientInfo.h"

class Server {
    u_short m_port;
    SOCKET m_listeningSocket;
    std::vector<SOCKET> m_clientSockets;
    std::vector<FClientInfo> m_clients;
    std::vector<std::thread> m_clientThreads;
    std::atomic<bool> m_serverRunning;
    
public:
    Server(u_short port);
    ~Server();

    void start();
    void stop();

private:
    static void CopyReceivedString(const char* source, char* destination, size_t destSize);
    void handleClient(SOCKET clientSocket);
    static std::vector<char> receiveData(SOCKET clientSocket, int dataSize);
    static char* receiveString(SOCKET clientSocket);
    void acceptClients();
    void processCommands();
};
