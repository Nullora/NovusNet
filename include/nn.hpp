#pragma once
#include <thread>
#include<string>
extern int client_fd;
extern bool clientConnected;
//Initialization

//Connecting
void runServer(int port);
int runClient(std::string ip, int port);

//After connection
void sendMsg(std::string msg,int id);
std::string recvMsg(int id);