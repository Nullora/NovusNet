#include "nn.hpp"
#include <iostream>
#include <chrono>

int main(){
    runServer(9090);
    while(true){
        onMessage([](int clientFD, std::string msg){
            std::cout << "Client " << clientFD << ": " << msg << "\n";
        });
    }
}