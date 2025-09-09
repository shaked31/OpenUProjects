//
// Created by Shaked Pollak on 8/28/2025.
//

#include "Server.h"
#include "ClientHandler.h"
#include <thread>

Server::Server(boost::asio::io_context& context, const unsigned short int port):
    a(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {} // Constructor

[[noreturn]] void Server::run() {
    // [[noreturn]] tells the compiler that this function will not return control to it's owner
    try {

        while (true) {
            boost::asio::ip::tcp::socket sock = a.accept();

            std::thread([sock = std::move(sock)]() mutable {
                ClientHandler cl;
                cl.start_session(std::move(sock));
            }).detach();
        }
    }

    catch (const boost::system::system_error& e) {
        if (e.code() == boost::asio::error::eof) {
            std::cout << "Client disconnected (EOF)" << std::endl;
        }
        else {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }
}
