#include "nn.hpp"
#include <string>
#include <map>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <netinet/tcp.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <fcntl.h>   // For O_RDONLY, etc.
#include <cerrno>    // For errno
#include<fstream>
#include <endian.h>
#include <sys/types.h>
#include <sys/stat.h>

int client_fd;                   // most recently connected client fd
std::map<int, SSL*> clients;      // all clients
int clients_index = 0;           // increment each time client connects
std::function<void(int, std::string)> messageCallback;
SSL_CTX* ssl_ctx = nullptr;
SSL* client_ssl = nullptr;

void onMessage(std::function<void(int, std::string)> callback){
    messageCallback = callback;
}

// spawns a background thread that accepts incoming connections.
// each accepted client gets their own recv thread.
void runServer(int port, std::string password) {
    // initialize OpenSSL and load certificates
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM)) {
        ERR_print_errors_fp(stderr);
    }
    if (!SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM)) {
        ERR_print_errors_fp(stderr);
    }
    ssl_ctx = ctx;
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

    std::thread([server_fd,password]() {
        while (true) {
            sockaddr_in client_addr{};
            socklen_t len = sizeof(client_addr);
            // block until a new client connects
            client_fd = accept(server_fd, (sockaddr*)&client_addr, &len);
            if (client_fd < 0) continue;
            //wrap fd in ssl
            SSL* ssl = SSL_new(ssl_ctx);
            SSL_set_fd(ssl, client_fd);
            if (SSL_accept(ssl) <= 0) {
                ERR_print_errors_fp(stderr);
                continue;
            }
            //check password (Access control)
            // register the new client
            clients_index++;
            clients[clients_index] = ssl;
            //check password (Access control)
            std::string recvdp = recvMsg(clients_index);
            if(recvdp!=password){
                std::cout << "KICKED (wrong password): " << inet_ntoa(client_addr.sin_addr) << "\n";
                SSL_shutdown(clients[clients_index]);
                SSL_free(clients[clients_index]);
                close(client_fd);
                clients.erase(clients_index);
            }else{
                std::cout << "CONNECTED: " << inet_ntoa(client_addr.sin_addr) << "\n";
                sendMsg(std::to_string(clients_index),clients_index);
                // stabilize client_index to pass to thread
                int new_ci = clients_index;
                int new_fd = client_fd;
                SSL* new_ssl = ssl;
                std::thread([new_ci,new_ssl,new_fd]() {
                    while (true) {
                        std::string msg = recvMsg(new_ci);
                        if (msg=="EXITED(C-178)"){
                            std::cout<<"client "<<new_ci<<" disconnected, cleaning.\n";
                            SSL_shutdown(clients[new_ci]);
                            SSL_free(clients[new_ci]);
                            close(new_fd);
                            clients.erase(new_ci);
                            break;
                        }
                        if (messageCallback) messageCallback(new_ci, msg);
                    }
                }).detach();
            }
        }
    }).detach();
}


int runClient(std::string ip, int port,std::string password) {
    int client = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr);

    if (connect(client, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("connect failed");
        return -1;
    }
    SSL_CTX* client_ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_verify(client_ctx, SSL_VERIFY_NONE, nullptr);
    SSL* ssl = SSL_new(client_ctx);
    SSL_set_fd(ssl, client);
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    client_ssl = ssl;
    sendMsg(password,client);
    std::cout << "Request sent\n";
    return client;
}


void sendMsg(std::string msg, int id) {
    SSL* ssl = (clients.count(id)) ? clients[id] : client_ssl;
    int msglength = msg.size();
    uint32_t msglengthC = htonl(msglength); 
    int bytesL = msglength;
    int bytesS = 0;
    int result=0;

    result = SSL_write(ssl, &msglengthC, sizeof(msglengthC));
    if (result < 0) {
        perror("send failed");
    }
    while (bytesL > 0) {
        result = SSL_write(ssl, msg.c_str() + bytesS, bytesL);
        if (result < 0) {
            perror("send failed");
            break;
        }
        bytesS += result;
        bytesL -= result;
    }
}

std::string recvMsg(int id) {
    SSL* ssl = (clients.count(id)) ? clients[id] : client_ssl;
    uint32_t msgL_htonl;
    int result=0;

    result = SSL_read(ssl, &msgL_htonl, sizeof(msgL_htonl));
    if (result < 0) {
        perror("recv failed");
        return "EXITED(C-178)";
    }
    int msgL = ntohl(msgL_htonl);
    int bytesR = 0;
    int bytesL = msgL;
    std::string msg(msgL, 0);   

    while (bytesL > 0) {
        result = SSL_read(ssl, msg.data() + bytesR, bytesL);
        if (result <= 0) {
            perror("recv failed");
            return "EXITED(C-1)";
        }
        bytesR += result;
        bytesL -= result;
    }

    return msg;
}

//NFTP
bool sendFile(std::string filepath, int id){
    SSL* ssl = (clients.count(id)) ? clients[id] : client_ssl;
    std::string filepathSTR = filepath;
    char buffer[16384];
    int fd = open(filepath.c_str(),O_RDONLY);
    if(fd<0) return false;
    struct stat st;
    fstat(fd,&st);
    uint64_t size = st.st_size;
    uint64_t netsize = htobe64(size);
    SSL_write(ssl,&netsize,sizeof(netsize));
    std::string filename = filepathSTR.substr(filepathSTR.find_last_of("/\\") + 1);
    sendMsg(filename,id);
    uint64_t bytesL = size;
    uint64_t bytesS = 0;
    int s=0;
    int result=0;
    while(bytesL>0){
        result = read(fd,buffer,sizeof(buffer));
        if(result<0) return false;
        s = SSL_write(ssl,buffer,result);
        bytesS += s;
        bytesL -= s;
    }
    return true;
}
bool recvFile(std::string folderpath, int id){
    SSL* ssl = (clients.count(id)) ? clients[id] : client_ssl;
    char buffer[16384];
    uint64_t netsize;
    if(SSL_read(ssl,&netsize,sizeof(netsize))<=0){
        perror("file recv failed");
        return false;
    }
    uint64_t filesize =  be64toh(netsize);
    std::string filename = recvMsg(id);
    uint64_t bytesL = filesize;
    uint64_t bytesR = 0;
    int s=0;
    int result=0;
    std::string fullpath = folderpath + "/" + filename;
    int outfd = open(fullpath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(outfd < 0) return false;
    while(bytesL>0){
        result = SSL_read(ssl, buffer, std::min(bytesL, (uint64_t)sizeof(buffer)));
        if(result<0) return false;
        bytesL -= result;
        bytesR += result;
        write(outfd, buffer, result);
    }
    close(outfd);
    return true;
}