//
// Created by Shaked Pollak on 8/29/2025.
//

#ifndef SERVERRESPONSES_H
#define SERVERRESPONSES_H
#include "ProtocolEnums.h"
#include <string>

#pragma pack(push, 1)
struct Response {
    std::uint8_t version;
    Status status;
    Response(const std::uint8_t version, const Status status)
        : version(version), status(status) {}
    Response() = default;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PartialFileResponse : Response {
    std::uint16_t name_len;
    std::string filename;
    PartialFileResponse(const std::uint8_t version, const Status status, const std::uint16_t name_len , const std::string &filename)
        : Response(version, status), name_len(name_len), filename(filename) {}
    PartialFileResponse();
};
#pragma pack(pop)

#pragma pack(push, 1)
struct FullFileResponse : PartialFileResponse {
    std::uint32_t size;
    FullFileResponse(const std::uint8_t version, const Status status,
    const std::uint16_t name_len, const std::string &filename, const std::uint32_t size)
        : PartialFileResponse(version, status, name_len, filename),
            size(size) {}

    FullFileResponse();
};
#pragma pack(pop)

#endif //SERVERRESPONSES_H
