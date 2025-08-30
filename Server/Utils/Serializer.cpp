//
// Created by Shaked Pollak on 8/29/2025.
//

#include "Serializer.h"

vector<uint8_t> Serializer::serializeResponse(const FullFileResponse &res) {
    vector<uint8_t> buffer = {}; // build a buffer that each element is 1 byte
    buffer.push_back(res.version);

    const uint16_t statusVal = static_cast<uint16_t>(res.status);
    buffer.push_back(statusVal & 0xFF); // take lower byte
    buffer.push_back((statusVal >> 8) & 0xFF); // takes upper byte

    buffer.push_back(res.name_len & 0xFF); // take lower byte
    buffer.push_back((res.name_len >> 8) & 0xFF); // takes upper byte

    buffer.insert(buffer.end(), res.filename.begin(), res.filename.end()); // insert filename bytes in 1 call

    buffer.push_back(res.size & 0xFF);
    buffer.push_back((res.size >> 8) & 0xFF);
    buffer.push_back((res.size >> 16) & 0xFF);
    buffer.push_back((res.size >> 24) & 0xFF);
    return buffer;
}
