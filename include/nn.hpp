#pragma once
#include<string>
extern int client_fd;
//Initialization

//Connecting
void runServer(int port);
int runClient(const char* ip, int port);

//After connection
void sendMsg(std::string msg);
std::string recvMsg();