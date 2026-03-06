#include "nn.hpp"
#include <string>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <netinet/tcp.h>

// global state
int client_fd;                   // most recently connected client fd
std::map<int, int> clients;      // all clients
bool clientConnected = false;    // least one client is connected
int clients_index = 0;           // incremented with each new connection

// spawns a background thread that accepts incoming connections.
// each accepted client gets their own recv thread.
void runServer(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 32);
    std::cout << "Server listening on port " << port << "\n";

    std::thread([server_fd]() {
        while (true) {
            sockaddr_in client_addr{};
            socklen_t len = sizeof(client_addr);

            // block until a new client connects
            client_fd = accept(server_fd, (sockaddr*)&client_addr, &len);
            if (client_fd < 0) continue;

            // Register the new client
            clients_index++;
            clients[clients_index] = client_fd;
            std::cout << "CONNECTED: " << inet_ntoa(client_addr.sin_addr) << "\n";
            clientConnected = true;

            // Capture fd by value so it's stable even if client_fd changes
            int new_fd = client_fd;
            std::thread([new_fd]() {
                // Loop until client disconnects (recvMsg returns empty on disconnect)
                while (true) {
                    std::string msg = recvMsg(new_fd);
                    if (msg=="EXITED(C-1)") break;
                    std::cout << msg << '\n';
                }
            }).detach();
        }
    }).detach();
}


int runClient(std::string ip, int port) {
    int client = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr);

    if (connect(client, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("connect failed");
        return -1;
    }

    std::cout << "Connection success\n";
    return client;
}


void sendMsg(std::string msg, int id) {
    int msglength = msg.size();
    uint32_t msglengthC = htonl(msglength); 
    int bytesL = msglength;
    int bytesS = 0;
    int result=0;

    result = send(id, &msglengthC, sizeof(msglengthC), 0);
    if (result < 0) {
        perror("send failed");
    }
    while (bytesL > 0) {
        result = send(id, msg.c_str() + bytesS, bytesL, 0);
        if (result < 0) {
            perror("send failed");
            break;
        }
        bytesS += result;
        bytesL -= result;
    }
}

std::string recvMsg(int id) {
    uint32_t msgL_htonl;
    int result=0;
    int client = clients[id];  // unused. kept for future use

    result = recv(id, &msgL_htonl, sizeof(msgL_htonl), 0);
    if (result <= 0) {
        perror("recv failed");
        return "EXITED(C-1)";
    }
    int msgL = ntohl(msgL_htonl);
    int bytesR = 0;
    int bytesL = msgL;
    std::string msg(msgL, 0);   

    while (bytesL > 0) {
        result = recv(id, msg.data() + bytesR, bytesL, 0);
        if (result <= 0) {
            perror("recv failed");
            return "EXITED(C-1)";
        }
        bytesR += result;
        bytesL -= result;
    }

    return msg;
}