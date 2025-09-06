//
// Created by Shaked Pollak on 8/28/2025.
//

#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <cstdint>


enum class OpCode : uint8_t {
    SAVE_FILE = 100,
    RETRIEVE_FILE = 200,
    DELETE_FILE = 201,
    LIST_FILES = 202
};

enum class Status : uint16_t {
    FILE_RETRIEVED_OK = 210,
    LIST_FILES_OK = 211,
    FILE_OPS_OK = 212,
    DELETE_OK = 212,
    FILE_NOT_EXISTS_ERR = 1001,
    NO_FILES_IN_SERVER_ERR = 1002,
    GENERAL_ERR = 1003
};
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
struct Response {
    uint8_t version;
    Status status;
    Response(const uint8_t version, const Status status)
        : version(version), status(status) {}
    Response() = default;
};
#pragma pack(pop)


#endif //PROTOCOL_H
