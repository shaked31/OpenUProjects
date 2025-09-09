#include <boost/asio.hpp>
#include <string>
#include <iostream>
#include <filesystem>

#include "Network/Server.h"



int main() {
    const int port = 5000;
    try {
        boost::asio::io_context context;
        Server server(context, port);
        server.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << std::endl;
    }
}