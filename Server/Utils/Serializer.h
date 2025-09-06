//
// Created by Shaked Pollak on 8/29/2025.
//

#ifndef SERIALIZER_H
#define SERIALIZER_H
#include <cstdint>
#include <vector>

#include "../Network/Protocols/Protocol.h"
#include "../Network/Protocols/ServerResponses.h"

using namespace std;

class Serializer {
public:
    vector<uint8_t> serializeFullFileResponse(const FullFileResponse& res);
    vector<uint8_t> serializeResponse(const Response& res);
    vector<uint8_t> serializePartialResponse(const PartialFileResponse& res);

    void deserializeResponse(const vector<uint8_t>& buffer, Response& res);

    vector<uint8_t> serializeRequest(const Request& res);
    void deserializeRequest(const vector<uint8_t>& buffer, Request& req);
};



#endif //SERIALIZER_H
