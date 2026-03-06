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