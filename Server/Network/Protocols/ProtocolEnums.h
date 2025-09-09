//
// Created by Shaked Pollak on 8/28/2025.
//

#ifndef PROTOCOL_H
#define PROTOCOL_H

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

#endif //PROTOCOL_H
