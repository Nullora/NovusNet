# NovusNet

NovusNet is a lightweight networking library designed to simplify client server communication. It allows developers to make secure connections and exchange data in just a few lines of code without the complexity of larger libraries such as Boost.Asio or others.

NovusNet abstracts away low-level socket handling and encryption, allowing you to focus on building your application instead of managing networking boilerplate.

# Key Features
- Get a client and server communicating in under 10 lines of code!
- Native encryption for all data transmission
- NFTP (Novus File Transfer Protocol): Send and receive files of any size with a single function call
- No unnecessary overhead or complexity
- Designed for ease of use and rapid integration
Project Status

#  Early Development

- APIs and syntax may change frequently

- Bugs are expected; feedback and reports are appreciated!

- NFTP is not yet fully stress-tested with large files

- Currently Linux-only (Windows support planned once stable)

# Motivation

Networking can be difficult to approach, especially for beginners. Setting up sockets, handling errors, and implementing encryption often requires significant effort and repetition across projects.

NovusNet was created to eliminate that friction by providing a simple, reusable solution for secure communication. Whether you're learning networking or building a small project, NovusNet allows you to move faster and focus on what matters.

# Contributors

- [Nullora](https://github.com/Nullora) (Owner/Dev)

- [intellectualtheft](https://github.com/intellectualtheft) (Dev)

- [droidcom](https://github.com/droidcom) (Writer)
# Usage Overview

Refer to documentation.md for a full API reference. Below is a simplified example:

# Server Example:
```cpp
#include "nn.hpp"
#include <iostream>
#include <chrono>

int main() {
    runServer(9090, "YOUR_PASSWORD");

    onMessage([](int clientN, std::string msg) {
        std::cout << "Client " << clientN << ": " << msg << "\n";
    });

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
```
# Client Example:
```cpp
#include "nn.hpp"
#include <iostream>

int main() {
    std::string msg;

    int client = runClient("127.0.0.1", 9090, "YOUR_PASSWORD");

    msg = recvMsg(client);
    int clientID = std::stoi(msg);

    while (true) {
        std::getline(std::cin, msg);
        sendMsg(msg, clientID);
    }

    return 0;
}
```
# Example Projects

- NovusSync: File sync example (primarily for testing and demonstration)

# Installation

## 1. Clone the repository:
```bash
git clone <repository-url>
```
## 2. Copy required files:
- include/nn.hpp → your project’s include directory
- src/nn.cpp → your project’s source directory

## 3. Include the library:
```bash
#include "nn.hpp"
```
## 4. If using CMake:
```bash
add_executable(PROJECTNAME
    src/yourfile.cpp
    src/nn.cpp
)

target_include_directories(PROJECTNAME PRIVATE include)

find_package(OpenSSL REQUIRED)
target_link_libraries(PROJECTNAME OpenSSL::SSL OpenSSL::Crypto)
```
## 5. Generate encryption keys:
```bash
chmod +x gen.sh
./gen.sh
```
# Notes:
- Ensure key.pem and cert.pem are present in your build directory when running the server
- Some IDEs may incorrectly highlight #include "nn.hpp" as an error so dont mind it.

# Note
If anyone can help me maintain this, please do. It's very hard maintaining it fully on my own although nobody uses it. Pull requests or discussions in issues are heavily appreciated!!

**Made by Mehdi B.R (Nullora @ Novus)**
