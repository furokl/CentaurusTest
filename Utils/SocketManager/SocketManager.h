#pragma once

#include <string>
#include <vector>
#include <winsock2.h>

class SocketManager {
public:
    static void sendData(SOCKET sock, const std::string& str);
    static void sendData(SOCKET sock, const std::vector<char>& data);
    static std::vector<char> receiveData(SOCKET clientSocket, int dataSize);
    static std::string receiveString(SOCKET clientSocket);
};
