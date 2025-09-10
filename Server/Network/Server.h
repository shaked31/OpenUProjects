//
// Created by Shaked Pollak on 8/28/2025.
//

#ifndef REQUEST_H
#define REQUEST_H

#include <asio.hpp>


class Server {
public:
    Server(asio::io_context& context, unsigned short int port); // constructor
    [[noreturn]] void run();

private:
    asio::ip::tcp::acceptor a;
};


#endif //REQUEST_H
