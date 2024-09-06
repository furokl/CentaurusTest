#pragma once

#include <winsock2.h>
#include <vector>
#include <thread>
#include <atomic>

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
    void handleClient(SOCKET clientSocket);
    void acceptClients();
    void processCommands();
};
