//
// Created by Shaked Pollak on 8/30/2025.
//

#include "GeneralException.h"

GeneralException::GeneralException(const string& msg)
    : message(msg) {
    res.version = VERSION;
    res.status = Status::GENERAL_ERR;
}

const char* GeneralException::what() const noexcept {
    return message.c_str();
}

void GeneralException::sendGeneralErrorResponse(tcp::socket sock) {
    Serializer serObj;
    vector<uint8_t> serialized = serObj.serializeResponse(res);

    boost::asio::write(sock, boost::asio::buffer(serialized.data(), serialized.size()));

    cout << "Response number " <<  int(res.status) << " was sent to client" << endl << endl;

}