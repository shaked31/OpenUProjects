//
// Created by Shaked Pollak on 8/28/2025.
//

#include "Server.h"
#include "ClientHandler.h"

Server::Server(io_context& context, const unsigned short int port):
    a(context, tcp::endpoint(tcp::v4(), port)) {} // Constructor

[[noreturn]] void Server::run() {
    // [[noreturn]] tells the compiler that this function will not return control to it's owner
    try {
        while (true) {
            ClientHandler(a.accept());
        }
    }
    catch (const boost::system::system_error& e) {
        if (e.code() == error::eof) {
            cout << "Client disconnected (EOF)" << endl;
        }
        else {
            cerr << "Exception: " << e.what() << endl;
        }
    }
}
