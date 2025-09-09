//
// Created by Shaked Pollak on 8/29/2025.
//

#ifndef SERIALIZER_H
#define SERIALIZER_H
#include <cstdint>
#include <vector>

#include "../Network/Protocols/ServerResponses.h"


class Serializer {
public:
    std::vector<uint8_t> serializeFullFileResponse(const FullFileResponse& res);
    std::vector<uint8_t> serializeResponse(const Response& res);
    std::vector<uint8_t> serializePartialResponse(const PartialFileResponse& res);
};



#endif //SERIALIZER_H
