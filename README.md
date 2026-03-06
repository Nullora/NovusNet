![CMake Build](https://github.com/Nullora/NovusNet/actions/workflows/cmake-single-platform.yml/badge.svg)
![License](https://img.shields.io/badge/license-MIT-blue)
![Platform](https://img.shields.io/badge/platform-Linux-pink)
# NovusNet
Ever manually set up a project that required networking on your own? You know how hard it is, thats why i'm introducing NovusNet. NN gets a server and client talking in less than 10 lines of code. Built for for indie devs, beginners, and anyone who's project doesn't need the overkill and complexity that larger libraries like Boost.Asio bring.
# WARNING
- Any connection is not encrypted yet, expose this code to the internet at your own risk.
- This is still in super early development, expect occasional bugs, and make sure to report said bugs to me.
- No Windows support exists yet, this is mainly for Linux systems, I'll add Windows support when the Linux version is truly stable.
# Why I made this
Learning sockets as a beginner is rough. The setup is long, 
the errors are annoying, and even once you get it, you still have 
to do it all over again every new project.

NovusNet exists so you don't have to do any of that. Whether you're just starting 
out with networking or you're an experienced dev who doesn't want 
to write boilerplate over and over, NovusNet handles the 
setup and the later communication, so you can focus on actually shipping your product.

# Code examples
## Server

```cpp
#include "nn.hpp"
#include <iostream>
#include <chrono>

int main(){
    runServer(9090);
    while(true){
        onMessage([](int clientID, std::string msg){
            std::cout << "Client " << clientID << ": " << msg << "\n";
        });
    }
}
```
## Client
```cpp
#include "nn.hpp"
#include <iostream>

int main(){
    std::string msg;
    //runClient(ip,port) connects to a server
    int client = runClient("127.0.0.1", 9090);
    while(true){
        std::getline(std::cin,msg);
        //sendMsg(string msg) sends data as a string
        sendMsg(msg,client);
    }
    return 0;
}
```

 - These same snippets can be found at **src/client.cpp** and **src/server.cpp**
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
