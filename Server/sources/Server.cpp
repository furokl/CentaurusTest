#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 5050;
std::vector<SOCKET> clientSockets;
std::atomic<bool> serverRunning(true);
// int argc, char* argv[]

// Функция для обработки каждого клиента
void handleClient(SOCKET clientSocket) {
    char buf[4096];
    
    while (serverRunning) {
        ZeroMemory(buf, 4096);

        // Ожидаем данные от клиента
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error receiving data" << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        }

        // Выводим данные клиента
        std::cout << "Received: " << std::string(buf, 0, bytesReceived) << std::endl;

        // Проверяем запрос на скриншот
        if (std::string(buf, 0, bytesReceived) == "screenshot") {
            // Логика получения скриншота от клиента (принимаем файл)
            std::cout << "Requesting screenshot..." << std::endl;

            // Ожидаем бинарные данные скриншота
            char screenshotBuf[8192];  // Буфер для получения данных скриншота
            int screenshotBytesReceived = recv(clientSocket, screenshotBuf, 8192, 0);
            
            // Сохраняем скриншот в файл
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

    // Закрываем сокет
    closesocket(clientSocket);
}

int main() {
    // Инициализация Winsock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsOk = WSAStartup(ver, &wsData);
    if (wsOk != 0) {
        std::cerr << "Can't Initialize winsock! Quitting" << std::endl;
        return 1;
    }

    // Создаем сокет
    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) {
        std::cerr << "Can't create a socket! Quitting" << std::endl;
        WSACleanup();
        return 1;
    }

    // Привязываем IP-адрес и порт к сокету
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    hint.sin_addr.s_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    // Запускаем сокет на прослушивание
    listen(listening, SOMAXCONN);
    
    std::vector<std::thread> threads;

    // Цикл для обработки подключений клиентов
    std::thread clientThread([&](){
        while (serverRunning) {
            sockaddr_in client;
            int clientSize = sizeof(client);
            SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Invalid client socket" << std::endl;
                closesocket(listening);
                WSACleanup();
                return;
            }

            clientSockets.push_back(clientSocket);
            threads.push_back(std::thread(handleClient, clientSocket));

            std::cout << "Client connected!" << std::endl;
        }
    });
    
    // Командная строка для отправки команд клиентам
    std::string command;
    while (serverRunning) {
        std::cout << "Enter command (screenshot): ";
        std::cin >> command;

        if (command == "screenshot") {
            for (SOCKET sock : clientSockets) {
                send(sock, command.c_str(), command.size() + 1, 0);
            } 
        } else if (command == "exit" || command == "q") {
            serverRunning = false;
            for (SOCKET sock : clientSockets) {
                send(sock, command.c_str(), command.size() + 1, 0);
                closesocket(sock); // Закрываем все сокеты клиентов
            }
        }
    }


    // Ожидаем завершения клиентских потоков
    std::cerr << std::boolalpha << clientThread.joinable() << '\n';
    clientThread.detach();
    
    // Ожидаем завершения всех потоков
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Закрываем прослушивающий сокет
    closesocket(listening);
    
    // Очищаем Winsock
    WSACleanup();

    return 0;
}