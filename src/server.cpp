#include "nn.hpp"
#include <iostream>
#include <chrono>

int main(){
    runServer(9090,"PassTest");
    //"onMessage" returns clientN and msg of any received message from any client.
    onMessage([](int clientN, std::string msg){
        //more detailed logic can go on here depending on what you wanna do.
        std::cout << "Client " << clientN << ": " << msg << "\n";
    });
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}