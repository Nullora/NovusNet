![CMake Build](https://github.com/Nullora/NovusNet/actions/workflows/cmake-single-platform.yml/badge.svg)
![License](https://img.shields.io/badge/license-MIT-blue)
![Platform](https://img.shields.io/badge/platform-Linux-pink)
# NovusNet
Ever manually set up a project that required networking on your own? You know how hard it is, thats why i'm introducing NovusNet. NN gets a server and client talking in less than 10 lines of code. Built for for indie devs, beginners, and anyone who's project doesn't need the overkill and complexity that larger libraries like Boost.Asio bring. Fully encrypted communication between server and clients.
# WARNING
- Although encryption is now added, anyone can still connect, I'll add Access Control soon enough, if I take too long, make your own.
- This is still in super early development, expect occasional bugs, and make sure to report said bugs to me.
- No Windows support exists yet, this is mainly for Linux systems, I'll add Windows support when the Linux version is truly stable.
# Why I made this
Learning sockets as a beginner is rough. The setup is long, 
the errors are annoying, and even once you get it, you still have 
to do it all over again every new project. And to truly secure it, encrypting messages is hell.

NovusNet exists so you don't have to do any of that. Whether you're just starting 
out with networking or you're an experienced dev who doesn't want 
to write boilerplate over and over, NovusNet handles the 
setup and the later communication, so you can focus on actually shipping your product.

## Usage
Check **Documentation.md**, it has a detailed explanation of each function, and check out the link below for an example project to further help you understand.

# Code examples
- An actual example project can be found at [NovusChat](https://github.com/Nullora/NovusChat). It is a pure NovusNet program.
## Server

```cpp
#include "nn.hpp"
#include <iostream>
#include <chrono>

int main(){
    //runServer takes port number only: runServer(int port)
    runServer(9090);
    //"onMessage" returns clientNumber and msg of any received message from any client.
    onMessage([](int clientN, std::string msg){
        //more detailed logic can go on here depending on what you wanna do.
        std::cout << "Client " << clientN << ": " << msg << "\n";
    });
    //Chrono so it does no tank cpu usage and doesn't let main() return anything
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
```
## Client
```cpp
#include "nn.hpp"
#include <iostream>

int main(){
    std::string msg;
    //runClient(ip,port) connects to a server and returns an int used to address the server
    int client = runClient("127.0.0.1", 9090);
    //receive client id and assign it for later use
    msg = recvMsg(client);
    int clientFD = std::stoi(msg);
    std::cout<<clientFD<<'\n';
    while(true){
        std::getline(std::cin,msg);
        //sendMsg(string msg) sends data as a string
        sendMsg(msg,1);
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
find_package(OpenSSL REQUIRED)
target_link_libraries(PROJECTNAME OpenSSL::SSL OpenSSL::Crypto)
```
- Compile and check if it works, sometimes code editors highight the **#include"nn.hpp"** in red, so check incase it's a real error instead of the usual false alarm.
# Updates
- Encryption is now fully working as intended. make sure to have the key.pem and cert.pem in your build folder when you run your server.

**Made by Mehdi B.R (Nullora @ Novus)**
