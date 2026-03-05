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