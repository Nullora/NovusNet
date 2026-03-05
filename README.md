# NovusNet
NovusNet is a C++ library that helps programmers setup servers and clients easily, removing the unnecessary hassle of setting it all up manually.
# Installation
- Just clone the repo with "git clone ..."
- Copy the "include/nn.hpp" file into your own include file.
- Copy the "src/nn.cpp" file into your own src file.
- Add line **#include"nn.hpp"** at the top of your main.cpp
# Code examples
## Server

```cpp
#include"nn.hpp"
#include<iostream>
#include<string>
#include <chrono>
int main(){
    runServer(9090);
    //chrono so loop dosen't tank CPU usage
    while(!clientConnected){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    //after clientConnected, receive simple messages
    while(true){
        std::cout<<recvMsg()<<"\n";
    }
    return 0;
}
```
## Client
```cpp
#include "nn.hpp"
#include <iostream>

int main(){
    std::string msg;
    runClient("127.0.0.1", 9090);
    while(true){
        std::getline(std::cin,msg);
        sendMsg(msg);
    }
    return 0;
}
```

# Why I made this
I always had trouble working with sockets as a beginner, and even after learning how, it still bothers me how I have to do it manually, so i decided to make NovusNet.
NN is a tool that sets it all up for you in 5 lines of code, simplicity meets efficency. It manages the initial connection, and the communication between the server and client.
# WARNING
- Any connection is not encrypted yet, expose this code to the internet at your own risk.
- This is still in super early developement, expect occasional bugs, and make sure to report said bugs to me.
# Note
No Windows support exists yet, this mainly for Linux systems, I'll add Windows support when the Linux version is truly stable.
