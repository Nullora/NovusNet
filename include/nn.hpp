#pragma once
#include <thread>
#include<string>
#include <functional>
extern int client_fd;
extern bool clientConnected;
//Initialization
void onMessage(std::function<void(int, std::string)> callback);
//Connecting
void runServer(int port);
int runClient(std::string ip, int port);

//After connection
void sendMsg(std::string msg, int id);
std::string recvMsg(int id);