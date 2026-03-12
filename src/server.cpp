#include "nn.hpp"
#include <iostream>
#include <chrono>

int main(){
    runServer(9090,"PassTest");
    int n=1;
    if(!recvFile("/home/nullora/ls/downloads",n)) std::cout<<"recv Failed\n";
    //"onMessage" returns clientN and msg of any received message from any client.
    onMessage([n](int clientN, std::string msg){
        //more detailed logic can go on here depending on what you wanna do.
        std::cout << "Client " << clientN << ": " << msg << "\n";
        clientN = n;
    });
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}