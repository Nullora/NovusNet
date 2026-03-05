#include"nn.hpp"
#include<iostream>
#include<chrono>

int main(){
    runServer(9090);
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}