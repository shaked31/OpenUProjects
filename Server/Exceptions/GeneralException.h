//
// Created by Shaked Pollak on 8/30/2025.
//

#ifndef GENERALEXCEPTIONS_H
#define GENERALEXCEPTIONS_H

#include <exception>
#include <string>
#include <asio.hpp>
#include "../Utils/Serializer.h"

#define VERSION 1

class GeneralException : public std::exception {
private:
    std::string message;
    Response res;

public:
    GeneralException(const std::string& msg);
    const char* what() const noexcept override;
    void sendGeneralErrorResponse(asio::ip::tcp::socket sock);
};

#endif //GENERALEXCEPTIONS_H
