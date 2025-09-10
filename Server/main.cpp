#include <asio.hpp>
#include <iostream>
#include "Network/Server.h"


int main() {
    try {
        constexpr int port = 5000;
        asio::io_context context;
        Server server(context, port);
        server.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << std::endl;
    }
}