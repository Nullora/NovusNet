cmake_minimum_required(VERSION 3.10)
project(ls)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(l
    src/server.cpp
    src/nn.cpp
)
add_executable(c
    src/client.cpp
    src/nn.cpp
)

target_include_directories(s PRIVATE include)
target_include_directories(c PRIVATE include)
find_package(OpenSSL REQUIRED)
target_link_libraries(s OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(c OpenSSL::SSL OpenSSL::Crypto)