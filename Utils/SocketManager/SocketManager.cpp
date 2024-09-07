#include "SocketManager.h"

#include <iostream>

/**
 * Отправка строки: размер данных, сами данные.
 * @param sock Сокет
 * @param str Строка, которую необходимо отправить.
 */
void SocketManager::sendData(SOCKET sock, const std::string& str) {
    int strSize = static_cast<int>(str.size());
    std::vector<char> buffer(sizeof(strSize) + strSize);
    
    memcpy(buffer.data(), &strSize, sizeof(strSize));
    memcpy(buffer.data() + sizeof(strSize), str.data(), strSize);
    
    int totalBytesSent = 0;
    int dataSize = static_cast<int>(buffer.size());
    while (totalBytesSent < dataSize)
    {
        int bytesSent = send(sock, buffer.data() + totalBytesSent, dataSize - totalBytesSent, 0);
        if (bytesSent == SOCKET_ERROR)
        {
            std::cerr << "Failed to send data, error: " << WSAGetLastError() << std::endl;
            break;
        }
        totalBytesSent += bytesSent;
    }
}

/**
 * Отправка данных: размер данных, сами данные.
 * @param sock Сокет
 * @param data Данные, которые необходимо отправить.
 */
void SocketManager::sendData(SOCKET sock, const std::vector<char>& data) {
    int strSize = static_cast<int>(data.size());
    std::vector<char> buffer(sizeof(strSize) + strSize);
    
    memcpy(buffer.data(), &strSize, sizeof(strSize));
    memcpy(buffer.data() + sizeof(strSize), data.data(), strSize);
    
    int totalBytesSent = 0;
    int dataSize = static_cast<int>(buffer.size());
    while (totalBytesSent < dataSize)
    {
        int bytesSent = send(sock, buffer.data() + totalBytesSent, dataSize - totalBytesSent, 0);
        if (bytesSent == SOCKET_ERROR)
        {
            std::cerr << "Failed to send data, error: " << WSAGetLastError() << std::endl;
            break;
        }
        totalBytesSent += bytesSent;
    }
}

/**
 * Получить данные от клиента: размер данных, сами данные.
 * @param clientSocket  Сокет клиента.
 * @param dataSize      Размер данных для получения.
 * @return Вектор полученных данных.
 */
std::vector<char> SocketManager::receiveData(SOCKET clientSocket, int dataSize) {
    std::vector<char> buffer(dataSize);
    int totalBytesReceived = 0;
    
    while (totalBytesReceived < dataSize) {
        int bytesRead = recv(clientSocket, buffer.data() + totalBytesReceived, dataSize - totalBytesReceived, 0);
        if (bytesRead == SOCKET_ERROR) {
            std::cerr << "Error receiving data, error: " << WSAGetLastError() << std::endl;
            return {};
        }
        totalBytesReceived += bytesRead;
    }
    
    return buffer;
}

/**
 * Получить строку от клиента: размер строки, сама строка.
 * @param clientSocket Сокет клиента.
 * @return Полученная строка.
 */
std::string SocketManager::receiveString(SOCKET clientSocket) {
    int strSize;
    int sizeReceived = recv(clientSocket, reinterpret_cast<char*>(&strSize), sizeof(strSize), 0);
    if (sizeReceived == SOCKET_ERROR || strSize <= 0) {
        std::cerr << "Failed to receive string size, error: " << WSAGetLastError() << std::endl;
        return "";
    }

    std::vector<char> stringData = receiveData(clientSocket, strSize);
    if (stringData.empty()) {
        return "";
    }

    return std::string(stringData.begin(), stringData.end());
}