//
// Created by Shaked Pollak on 8/30/2025.
//

#ifndef GENERALEXCEPTIONS_H
#define GENERALEXCEPTIONS_H

#include <iostream>
#include <exception>
#include <string>
#include <boost/asio.hpp>
#include "../Network/Protocols/Protocol.h"
#include "../Utils/Serializer.h"

#define VERSION 1

using namespace std;
using boost::asio::ip::tcp;

class GeneralException : public std::exception {
private:
    string message;
    Response res;


public:
    GeneralException(const string& msg);
    const char* what() const noexcept override;
    void sendGeneralErrorResponse(tcp::socket sock);
};

#endif //GENERALEXCEPTIONS_H
