//
// Created by Shaked Pollak on 8/29/2025.
//

#ifndef CLIENTREQUESTS_H
#define CLIENTREQUESTS_H
#include <string>
#include "ProtocolEnums.h"


#pragma pack(push, 1)
struct Request {
    uint32_t uid;
    uint8_t version;
    OpCode op;
    Request(const uint32_t uid, const uint8_t version, const OpCode op)
        : uid(uid), version(version), op(op) {}
    Request(const Request& otherReq)
        : uid(otherReq.uid), version(otherReq.version), op(otherReq.op) {}
    Request() = default;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ListRequest : Request {
    ListRequest(const uint32_t uid, const uint8_t version)
        : Request(uid, version, OpCode::LIST_FILES) {}
    ListRequest(const Request& request)
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
    FileOpsRequest(const Request& otherReq, const uint16_t name_len, std::string filename)
        : Request(otherReq.uid, otherReq.version, otherReq.op),
        name_len(name_len), filename(std::move(filename)) {}

    FileOpsRequest(const FileOpsRequest& otherReq) : Request(otherReq.uid, otherReq.version, otherReq.op), name_len(otherReq.name_len), filename(std::move(otherReq.filename)) {}

    FileOpsRequest(const Request& req) : Request(req), name_len(0), filename("") {}
    FileOpsRequest() = default;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SaveFileRequest : FileOpsRequest {
    uint32_t size;
    SaveFileRequest(const uint32_t uid, const uint8_t version,
    const uint16_t name_len, const std::string& filename, const uint32_t size)
        : FileOpsRequest(uid, OpCode::SAVE_FILE, version, name_len, filename),
            size(size) {}

    SaveFileRequest(const Request& req, const uint16_t name_len, const std::string& filename, const uint32_t size)
        : FileOpsRequest(req, name_len, filename), size(size) {}

    SaveFileRequest(const FileOpsRequest& req) : FileOpsRequest(req), size(0) {}

    SaveFileRequest(const Request& req) : FileOpsRequest(req, 0, ""), size(0) {}

    SaveFileRequest() = default;
};
#pragma pack(pop)

#endif //CLIENTREQUESTS_H