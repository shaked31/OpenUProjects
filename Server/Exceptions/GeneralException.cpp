//
// Created by Shaked Pollak on 8/30/2025.
//

#include "GeneralException.h"
#include <iostream>


GeneralException::GeneralException(const std::string& msg)
    : message(msg) {
    res.version = VERSION;
    res.status = Status::GENERAL_ERR;
}

const char* GeneralException::what() const noexcept {
    return message.c_str();
}

void GeneralException::sendGeneralErrorResponse(asio::ip::tcp::socket sock) {
    Serializer serObj;
    std::vector<uint8_t> serialized = serObj.serializeResponse(res);

    asio::write(sock, asio::buffer(serialized.data(), serialized.size()));

    std::cout << "Response number " <<  int(res.status) << " was sent to client" << std::endl << std::endl;

}