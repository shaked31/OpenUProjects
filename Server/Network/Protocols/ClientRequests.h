//
// Created by Shaked Pollak on 8/29/2025.
//

#ifndef CLIENTREQUESTS_H
#define CLIENTREQUESTS_H
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "Protocol.h"

#pragma pack(push, 1)
struct ListRequest : Request {
    ListRequest(const uint32_t uid, const uint8_t version)
        : Request(uid, version, OpCode::LIST_FILES) {}
    ListRequest(const Request &request)
        : Request(request.uid, request.version, OpCode::LIST_FILES) {}
    ListRequest();
};

#pragma pack(push, 1)
struct FileOpsRequest : Request {
    uint16_t name_len;
    std::string filename;
    FileOpsRequest(const uint32_t uid, const OpCode op, const uint8_t version,
        const uint16_t name_len, std::string filename)
            : Request(uid, version, op),
                name_len(name_len),
                filename(std::move(filename)) {}

    FileOpsRequest();
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SaveFileRequest : FileOpsRequest {
    uint32_t size;
    std::vector<uint8_t> payload;
    SaveFileRequest(const uint32_t uid, const uint8_t version,
    const uint16_t name_len, const std::string &filename, const uint32_t size, std::vector<uint8_t> payload)
        : FileOpsRequest(uid, OpCode::SAVE_FILE, version, name_len, filename),
            size(size),
            payload(std::move(payload)) {}

    SaveFileRequest();
};
#pragma pack(pop)

#endif //CLIENTREQUESTS_H