//This library is made by @Nullora on Github. The link can be found here, and documentation aswell: https://github.com/Nullora/NovusNet
//NovusNet is a c++ networking library made to facilitate connection between devices while keeping it fast and secure.
//It's fully free and anyone can distribute/use it. im testing stuff rn.
//Last updated: 16/3/26
#pragma once
#include <thread>
#include<string>
#include <functional>
#include <map>
#include <openssl/ssl.h>
extern std::map<int, SSL*> clients; 
//Initialization
void onMessage(std::function<void(int, std::string)> callback);
//Connecting
void runServer(int port, std::string password);
int runClient(std::string ip, int port,std::string password);

//After connection
void sendMsg(std::string msg, int id);
std::string recvMsg(int id);

//NFTP
bool sendFile(std::string filepath, int id);
bool recvFile(std::string folderpath, int id);