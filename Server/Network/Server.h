//
// Created by Shaked Pollak on 8/28/2025.
//

#ifndef REQUEST_H
#define REQUEST_H

#include <boost/asio.hpp>
using namespace boost::asio;
using ip::tcp;


class Server {
public:
    Server(io_context& context, unsigned short int port); // constructor
    [[noreturn]] void run();

private:
    tcp::acceptor a;
};


#endif //REQUEST_H
