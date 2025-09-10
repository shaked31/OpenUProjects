cmake_minimum_required(VERSION 3.31)
project(Server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("C:/Users/hanoi/Desktop/CLION projects/MMN14/Server/include")

set(SOURCES
        main.cpp
        Network/Server.cpp
        Network/ClientHandler.cpp
        Utils/Serializer.cpp
        Exceptions/GeneralException.cpp
)

add_executable(Server ${SOURCES})

target_link_libraries(Server ws2_32)
