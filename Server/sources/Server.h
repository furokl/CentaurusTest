#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <thread>
#include <atomic>

class Server {
    int m_port;
    SOCKET m_listeningSocket;
    std::vector<SOCKET> m_clientSockets;
    std::vector<std::thread> m_clientThreads;
    std::atomic<bool> m_serverRunning;
    
public:
    Server(int port);
    ~Server();

    void start();
    void stop();

private:
    void handleClient(SOCKET clientSocket);
    void acceptClients();
    void processCommands();
};
