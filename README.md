# NovusNet
NovusNet is a C++ library that helps programmers setup servers and clients easily, removing the unnecessary hassle of setting it all up manually.
# Installation
- Just clone the repo with "git clone ..."
- Copy the "include/conn.hpp" file into your own include file.
- Copy the "src/conn.cpp" file into your own src file.
- Add line **#include"conn.hpp"** at the top of your main.cpp
# Usage
- Functions and all can be found in the GitHub Wiki.
# Why I made this
I always had trouble working with sockets as a beginner, and even after learning how, it still bothers me how I have to do it manually, so i decided to make NovusNet.
NN is a tool that sets it all up for you in 5 lines of code, simplicity meets efficency. It manages the initial connection, and the communication between the server and client.
# WARNING
- Any connection is not encrypted yet, expose this code to the internet at your own risk.
- This is still in super early developement, expect occasional bugs, and make sure to report said bugs to me.
# Note
No Windows support exists yet, this mainly for Linux systems, I'll add Windows support when the Linux version is truly stable.
