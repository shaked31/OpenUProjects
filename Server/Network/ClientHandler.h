//
// Created by Shaked Pollak on 8/28/2025.
//

#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H
#define VERSION 1


#include <boost/asio.hpp>
#include "Protocols/Protocol.h"
#include "Protocols/ClientRequests.h"
#include <iostream>
#include <filesystem>
#include <fstream>


using boost::asio::ip::tcp;
using namespace std;

class ClientHandler {
public:
    ClientHandler(tcp::socket sock);
    void start_session();

private:
    tcp::socket clientHandlerSock;
    void handle_list_req(Request req);
    string generateFilename();

    void handle_retreive_req();
    void handle_delete_req();
    void handle_save_req();
};



#endif //CLIENTHANDLER_H
