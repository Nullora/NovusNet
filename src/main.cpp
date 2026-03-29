//this snippet just opens a server and stops it from closing.
#include "nn.hpp"
#include <iostream>
int main(){
    std::string msg;
    runServer(9090,"PassTest");

    
    //this part stops it from reaching the "return 0" line.
    //the timer is so it does not eat up CPU power
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}