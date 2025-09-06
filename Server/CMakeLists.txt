cmake_minimum_required(VERSION 3.31)
project(Server)

set(CMAKE_CXX_STANDARD 20)

set(Boost_NO_BOOST_CMAKE ON)
set(Boost_NO_SYSTEM_PATHS OFF)

find_package(Boost REQUIRED COMPONENTS filesystem system)

set(SOURCES
    main.cpp
    Network/Server.cpp
    Network/ClientHandler.cpp
    Utils/Serializer.cpp
    Exceptions/GeneralException.cpp
)

add_executable(Server ${SOURCES})

target_link_libraries(Server PRIVATE Boost::filesystem Boost::system ws2_32)
