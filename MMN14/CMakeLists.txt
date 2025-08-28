cmake_minimum_required(VERSION 4.0)
project(MMN14)

set(CMAKE_CXX_STANDARD 20)
find_package(Boost REQUIRED COMPONENTS filesystem system)

add_executable(MMN14 main.cpp)
target_link_libraries(MMN14 PRIVATE Boost::filesystem Boost::system ws2_32)
