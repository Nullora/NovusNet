#include"nn.hpp"
#include<string>
#include<sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include<cstring>
#include<iostream>
#include <netinet/tcp.h>
using namespace std;
int client_fd;
void runServer(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server_fd,(sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 32);

    std::cout << "Server listening on port " << port << "\n";

    while (true) {
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        client_fd = accept(server_fd, (sockaddr*)&client_addr, &len);
        if (client_fd < 0) continue;
        std::cout << "CONNECTED:  " << inet_ntoa(client_addr.sin_addr) << "\n";
        sendMsg("ACCEPTED");
    }
}
int runClient(const char* ip, int port){
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
}
void sendMsg(std::string msg){
    int msglength = msg.size();
    uint32_t msglengthC = htonl(msglength);
    int bytesL = msglength;
    int bytesS = 0;
    send(client_fd,&msglengthC,sizeof(msglengthC),0);
    while(bytesL>0){
        int result = send(client_fd,msg.c_str()+bytesS,bytesL,0);
        if(result<0){
            perror("send failed");
            break;
        }
        bytesS += result;
        bytesL -= result;
    }
}
std::string recvMsg(){
    uint32_t msgL_htonl;
    recv(client_fd,&msgL_htonl,sizeof(msgL_htonl),0);
    int msgL = ntohl(msgL_htonl);
    int bytesR = 0;
    int bytesL = msgL;
    std::string msg(msgL, 0);
    while(bytesL>0){
        int result = recv(client_fd,msg.data()+bytesR,bytesL,0);
        if(result<0){
            perror("recv failed");
            break;
        }
        bytesR += result;
        bytesL -= result;
    }
    return msg;
}