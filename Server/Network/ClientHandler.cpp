//
// Created by Shaked Pollak on 8/28/2025.
//

#include "ClientHandler.h"

#include <windows.h>
#include <random>

#include "../Utils/Serializer.h"
#include "Protocols/ServerResponses.h"
#include "Protocols/Protocol.h"
#include "../Exceptions/GeneralException.h"
#include "Protocols/ClientRequests.h"

#define VERSION 1
#define BUFFER_SIZE 4096

using namespace std;
using namespace std::filesystem;
using namespace boost::asio;

const path BASE_DIR = L"C:\\backupsrv";


void ClientHandler::start_session(tcp::socket sock) {
    try {
        Request req{};
        read(sock, buffer(&req, sizeof(req)));
        if (req.version != VERSION) {
            throw GeneralException("Version of client not supported");
        }
        cout << "Request number " <<  static_cast<int>(req.op) << " from the user " <<  req.uid << " was received" << endl;

        if (req.op == OpCode::LIST_FILES)
            handle_list_req(std::move(sock), req);
        else {
            FileOpsRequest fileOpsReq(req);
            read(sock, buffer(&fileOpsReq.name_len, 2));
            fileOpsReq.filename.resize(fileOpsReq.name_len);
            read(sock, buffer(fileOpsReq.filename.data(), fileOpsReq.name_len));

            if (req.op == OpCode::RETRIEVE_FILE)
                handle_retrieve_req(std::move(sock), fileOpsReq);
            else if (req.op == OpCode::DELETE_FILE)
                handle_delete_req(std::move(sock), fileOpsReq);
            else if (req.op == OpCode::SAVE_FILE) {
                SaveFileRequest saveFileReq(fileOpsReq);

                cout << saveFileReq.filename << endl;

                read(sock, buffer(&saveFileReq.size, 4));
                saveFileReq.payload.resize(saveFileReq.size);

                read(sock, buffer(saveFileReq.payload.data(), saveFileReq.size));

                handle_save_req(std::move(sock), saveFileReq);
            }
            else {
                throw GeneralException("A non expected request was received");
            }
        }
    }
    catch (GeneralException& e) {
        cout << e.what() << endl;
        e.sendGeneralErrorResponse(std::move(sock));
    }
}

bool ClientHandler::folder_exists(const std::string &path) {
    WIN32_FIND_DATAA findFileData;
    const HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return false; // folder doesn't exist or inaccessible
    }
    FindClose(hFind);
    return true;
}

void ClientHandler::handle_retrieve_req(tcp::socket sock, const FileOpsRequest& req) {
    try {
        const path folderPath = BASE_DIR / to_string(req.uid);
        const path targetFilePath = safe_path(folderPath, req.filename);

        error_code ec;
        if (!folder_exists(folderPath.string())) {
            Response res{};
            res.version = VERSION;
            res.status = Status::NO_FILES_IN_SERVER_ERR;
            write(sock, buffer(&res, sizeof(res)));

            cout << "Response number " <<  static_cast<int>(res.status) << " was sent to client" << endl << endl;
            throw filesystem_error("user doesnt exist", ec);
        }

        ifstream file(targetFilePath, ios::binary);
        if (!file.is_open()) {
            PartialFileResponse partRes(VERSION, Status::FILE_NOT_EXISTS_ERR, req.name_len, req.filename);
            Serializer serObj;
            vector<uint8_t> serialized = serObj.serializePartialResponse(partRes);
            write(sock, buffer(serialized.data(), serialized.size()));

            cout << "Response number " <<  static_cast<int>(partRes.status) << " with file '" <<  partRes.filename << "' was sent to client" << endl << endl;
            throw filesystem_error("file doesnt exist", ec);
        }


        file.seekg(0, ios::end);
        uint32_t fileSize = file.tellg();
        file.seekg(0, ios::beg);

        FullFileResponse fullFileRes(VERSION, Status::FILE_RETRIEVED_OK, req.name_len,
        req.filename, fileSize);

        Serializer serObj;
        vector<uint8_t> serialized = serObj.serializeFullFileResponse(fullFileRes);
        write(sock, buffer(serialized.data(), serialized.size()));

        char payloadBuffer[BUFFER_SIZE];
        while (true) {
            file.read(payloadBuffer, sizeof(payloadBuffer)); // reads 4096 bytes from file to the buffer
            streamsize size = file.gcount();
            if (size <= 0) {
                break;
            }
            write(sock, buffer(payloadBuffer, file.gcount()));
        }
        file.close();
    }
    catch (const filesystem_error& e) {
        cout << e.what() << endl;

    }
}

void ClientHandler::handle_delete_req(tcp::socket sock, const FileOpsRequest& req) {
    try {
        const path targetUserPath = BASE_DIR / to_string(req.uid);
        if (!folder_exists(targetUserPath.string())) {
            // send no user exists response
            Response res{};
            res.version = VERSION;
            res.status = Status::NO_FILES_IN_SERVER_ERR;
            write(sock, buffer(&res, sizeof(res)));

            cout << "Response number " <<  static_cast<int>(res.status) << " was sent to client" << endl << endl;
        }
        const path fileTargetPath = safe_path(targetUserPath, req.filename);
        if (remove(fileTargetPath)) {
            cout << "File " << req.filename << " was deleted" << endl;
            PartialFileResponse partRes(VERSION, Status::FILE_OPS_OK, req.name_len, req.filename);
            Serializer serObj;
            vector<uint8_t> serialized = serObj.serializePartialResponse(partRes);
            write(sock, buffer(serialized.data(), serialized.size()));

            cout << "Response number " <<  static_cast<int>(partRes.status) << " with file '" <<  partRes.filename << "' was sent to client" << endl << endl;
        }
        else {
            PartialFileResponse partRes(VERSION, Status::FILE_NOT_EXISTS_ERR, req.name_len, req.filename);
            Serializer serObj;
            vector<uint8_t> serialized = serObj.serializePartialResponse(partRes);
            write(sock, buffer(serialized.data(), serialized.size()));

            cout << "Response number " <<  static_cast<int>(partRes.status) << " with file '" <<  partRes.filename << "' was sent to client" << endl << endl;
        }
    }
    catch (GeneralException& e) {
        cout << e.what() << endl;
        e.sendGeneralErrorResponse(std::move(sock));
    }
}

void ClientHandler::handle_save_req(tcp::socket sock, const SaveFileRequest& req) {
    try {
        const path userTargetPath = BASE_DIR / to_string(req.uid);
        create_directories(userTargetPath);

        const path filePath = safe_path(userTargetPath, req.filename);

        ofstream file(filePath, ios::binary | ios::trunc);
        if (!file.is_open()) {
            throw GeneralException("Couldn't open file for writing");
        }
        if (req.size == 0 || req.payload.empty()) {
            throw GeneralException("The payload is empty");
        }

        file.write(reinterpret_cast<const char*>(req.payload.data()), req.size);
        if (!file) {
            throw GeneralException("Couldn't write payload to file");
        }
        file.flush();
        file.close();

        PartialFileResponse partRes(VERSION, Status::FILE_OPS_OK, req.name_len, req.filename);
        Serializer serObj;
        vector<uint8_t> serialized = serObj.serializePartialResponse(partRes);
        write(sock, buffer(serialized.data(), serialized.size()));

        cout << "Response number " <<  static_cast<int>(partRes.status) << " with file '" <<  partRes.filename << "' was sent to client" << endl << endl;
    }
    catch (const filesystem_error& e) {
        cerr << e.what() << endl;
        Response res{};
        res.version = VERSION;
        res.status = Status::NO_FILES_IN_SERVER_ERR;
        write(sock, buffer(&res, sizeof(res)));

        cout << "Response number " <<  static_cast<int>(res.status) << " was sent to client" << endl << endl;
    }
    catch (GeneralException& e) {
        cerr << e.what() << endl;
        e.sendGeneralErrorResponse(std::move(sock));
    }
}



void ClientHandler::handle_list_req(tcp::socket sock, const Request& req) {
    const path tmpFilename = temp_directory_path() / "backupsrv" / (generateFilename() + ".txt");
    create_directories(tmpFilename.parent_path());
    try {
        ListRequest listReq(req);
        path clientPath = safe_path(BASE_DIR, to_string(listReq.uid));

        const vector<string> defaultFiles = {
            "backup.info",
            "server.info",
            "Request.py",
            "FilesReader.py",
            "client.py"
        };


        ofstream outNewFile(tmpFilename);
        if (!outNewFile.is_open()) {
            throw GeneralException("A general error occurred when trying to create a file");
            // general error, throw exception
        }

        for (const auto& entry : directory_iterator(clientPath)) {
            string currFileName = entry.path().filename().string();
            if (find(defaultFiles.begin(), defaultFiles.end(),
                    currFileName) == defaultFiles.end()) {
                outNewFile << currFileName << endl;
            }
        }

        outNewFile.flush();
        outNewFile.close();

        ifstream inNewFile(tmpFilename, ios::binary); // open file in binary mode
        if (!inNewFile.is_open()) {
            // general error, throw exception
            throw GeneralException("Couldn't open file for reading");
        }

        inNewFile.seekg(0, ios::end);
        uint32_t tmpFileSize =  inNewFile.tellg();
        inNewFile.seekg(0, ios::beg);

        uint16_t pathToNewTmpFileLength = static_cast<uint16_t>(tmpFilename.string().length());
        FullFileResponse fullFileRes(VERSION, Status::LIST_FILES_OK, pathToNewTmpFileLength,
            tmpFilename.string(), tmpFileSize);

        Serializer serObj;
        vector<uint8_t> serialized = serObj.serializeFullFileResponse(fullFileRes);
        write(sock, buffer(serialized.data(), serialized.size()));

        char payloadBuffer[BUFFER_SIZE];
        while (true) {
            inNewFile.read(payloadBuffer, sizeof(payloadBuffer));
            streamsize size = inNewFile.gcount();
            if (size <= 0) {
                break;
            }
            write(sock, buffer(payloadBuffer, inNewFile.gcount()));
        }
        inNewFile.close();

        cout << "Response number " <<  static_cast<int>(fullFileRes.status) << " with file '" <<  fullFileRes.filename << "' was sent to client" << endl << endl;

    }
    catch (const filesystem_error& e) {
        cerr << e.what() << endl;
        Response res{};
        res.version = VERSION;
        res.status = Status::NO_FILES_IN_SERVER_ERR;
        write(sock, buffer(&res, sizeof(res)));

        cout << "Response number " <<  static_cast<int>(res.status) << " was sent to client" << endl << endl;
    }

    catch (GeneralException& e) {
        cerr << e.what() << endl;
        e.sendGeneralErrorResponse(std::move(sock));
    }

    // clientHandlerSock.close();
}

// void save_payload_to_file(tcp::socket sock, path& filePath, uint32_t fileSize) {
//     ofstream file()
// }

string ClientHandler::generateFilename() {
    constexpr size_t length = 32;
    random_device rand;
    mt19937 gen(rand());
    uniform_int_distribution<> distrib(0, 61);

    string result;
    result.reserve(length);

    constexpr int ASCII_DIGIT = 10;
    constexpr int ASCII_UPPERCASE = 36;
    for (size_t i=0 ; i<length ; i++) {
        const int num = distrib(gen);
        if (num < ASCII_DIGIT) {
            // 0-9  char
             result += '0' + num;
        }
        else if (num < ASCII_UPPERCASE) {
            // A-Z char
            result += 'A' + num - ASCII_DIGIT;
        }
        else {
            // a-z char
            result += 'a' + num - ASCII_UPPERCASE;
        }
    }

    return result;
}

path ClientHandler::safe_path(const path& baseDir, const string& filename) {
    const path targetFilePath = weakly_canonical(baseDir / filename);
    // weakly_canonical resolves folderPath/filename into absolute path (without . and ..)
    if (targetFilePath.native().find(BASE_DIR.native()) != 0) {
        // if the base path of targetFilePath is different from the base path of folderPath
        // meaning there that an attempt for path traversal
        throw GeneralException("Invalid filename (path traversal attempt)");
    }
    return targetFilePath;
}
