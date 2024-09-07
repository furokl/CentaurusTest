#pragma once

#include <winsock2.h>
#include <vector>
#include <thread>
#include <atomic>
#include <string>

struct FClientInfo;

class Server {
    u_short m_port;
    SOCKET m_listeningSocket;
    std::vector<FClientInfo> m_clients;
    std::vector<std::thread> m_clientThreads;
    std::atomic<bool> m_serverRunning;
    
public:
    Server(u_short port);
    ~Server();

    void start();
    void stop();

private:
    static void CopyReceivedString(const std::string& source, char* destination, size_t destSize);
    void handleClient(SOCKET clientSocket, FClientInfo& clientInfo);
    void acceptClients();
    void processCommands();
};
