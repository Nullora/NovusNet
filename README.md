![CMake Build](https://github.com/Nullora/NovusNet/actions/workflows/cmake-single-platform.yml/badge.svg)
![License](https://img.shields.io/badge/license-MIT-blue)
# NovusNet
NovusNet is a C++ library that helps programmers setup and manage servers and clients easily, removing the unnecessary hassle of setting it all up manually.
# Installation
- Just clone the repo with "git clone ..."
- Copy the "include/nn.hpp" file into your own include file.
- Copy the "src/nn.cpp" file into your own src file.
- Add line **#include"nn.hpp"** at the top of your main.cpp
- Don't forget to include the new files in your CMakeLists.txt if you have one:

```cpp
add_executable(PROJECTNAME
    src/main.cpp
    src/nn.cpp
)
target_include_directories(PROJECTNAME PRIVATE include)
```

# Code examples
## Server

```cpp
#include"nn.hpp"
#include<iostream>
#include <chrono>
int main(){
    runServer(9090);
    //chrono so loop doesn't tank CPU usage
    while(!clientConnected){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    //after clientConnected, receive simple messages
    while(true){
        //recvMsg() returns a string.
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
    //runClient(ip,port) connects to a server
    runClient("127.0.0.1", 9090);
    while(true){
        std::getline(std::cin,msg);
        //sendMsg(string msg) sends data as a string
        sendMsg(msg);
    }
    return 0;
}
```

# Why I made this
I always had trouble working with sockets as a beginner, and even after learning how, it still bothers me how I have to do it manually, so i decided to make NovusNet.
NN is a tool that sets it all up for you in 5 lines of code, simplicity meets efficiency. It manages the initial connection, and the later communication between the server and client.
# WARNING
- Any connection is not encrypted yet, expose this code to the internet at your own risk.
- This is still in super early development, expect occasional bugs, and make sure to report said bugs to me.
- As of right now, NovusNet servers can only handle 1 client at a time, don't expect much.
# Note
No Windows support exists yet, this is mainly for Linux systems, I'll add Windows support when the Linux version is truly stable.
