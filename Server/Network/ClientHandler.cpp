//
// Created by Shaked Pollak on 8/28/2025.
//

#include "ClientHandler.h"

#include <random>
#include <windows.h>

#include "../Exceptions/GeneralException.h"
#include "../Utils/Serializer.h"
#include "Protocols/ClientRequests.h"
#include "Protocols/ProtocolEnums.h"
#include "Protocols/ServerResponses.h"

#define VERSION 1
#define BUFFER_SIZE 4096
#define NAME_LEN_BYTES 2
#define FILE_SIZE_BYTES 4

const std::filesystem::path BASE_DIR = L"C:\\backupsrv";

void ClientHandler::start_session(boost::asio::ip::tcp::socket sock) {
    try {
        Request req{};
        boost::asio::read(sock, boost::asio::buffer(&req, sizeof(req)));
        if (req.version != VERSION) {
            throw GeneralException("Version of client not supported");
        }
        std::cout << "Request number " <<  static_cast<int>(req.op) << " from the user " <<  req.uid << " was received" << std::endl;

        if (req.op == OpCode::LIST_FILES)
            handle_list_req(sock, req);
        else {
            FileOpsRequest fileOpsReq(req);
            boost::asio::read(sock, boost::asio::buffer(&fileOpsReq.name_len, NAME_LEN_BYTES));
            fileOpsReq.filename.resize(fileOpsReq.name_len);
            boost::asio::read(sock, boost::asio::buffer(fileOpsReq.filename.data(), fileOpsReq.name_len));

            if (req.op == OpCode::RETRIEVE_FILE)
                handle_retrieve_req(sock, fileOpsReq);
            else if (req.op == OpCode::DELETE_FILE)
                handle_delete_req(sock, fileOpsReq);
            else if (req.op == OpCode::SAVE_FILE) {
                SaveFileRequest saveFileReq(fileOpsReq);
                boost::asio::read(sock, boost::asio::buffer(&saveFileReq.size, FILE_SIZE_BYTES));
                handle_save_req(sock, saveFileReq);
            }
            else {
                throw GeneralException("A non expected request was received");
            }
        }
    }
    catch (GeneralException& e) {
        std::cout << e.what() << std::endl;
        e.sendGeneralErrorResponse(std::move(sock));
    }
}


void ClientHandler::handle_retrieve_req(boost::asio::ip::tcp::socket& sock, const FileOpsRequest& req) {
    try {
        const std::filesystem::path folderPath = BASE_DIR / std::to_string(req.uid);
        const std::filesystem::path targetFilePath = safe_path(folderPath, req.filename);

        std::error_code ec;
        if (!folder_exists(folderPath.string())) {
            Response res{};
            res.version = VERSION;
            res.status = Status::NO_FILES_IN_SERVER_ERR;
            boost::asio::write(sock, boost::asio::buffer(&res, sizeof(res)));

            std::cout << "Response number " <<  static_cast<int>(res.status) << " was sent to client" << std::endl << std::endl;
            throw std::filesystem::filesystem_error("user doesnt exist", ec);
        }

        std::ifstream file(targetFilePath, std::ios::binary);
        if (!file.is_open()) {
            PartialFileResponse partRes(VERSION, Status::FILE_NOT_EXISTS_ERR, req.name_len, req.filename);
            Serializer serObj;
            std::vector<uint8_t> serialized = serObj.serializePartialResponse(partRes);
            boost::asio::write(sock, boost::asio::buffer(serialized.data(), serialized.size()));

            std::cout << "Response number " <<  static_cast<int>(partRes.status) << " with file '" <<  partRes.filename << "' was sent to client" << std::endl << std::endl;
            throw std::filesystem::filesystem_error("file doesnt exist", ec);
        }

        file.seekg(0, std::ios::end);
        uint32_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        FullFileResponse fullFileRes(VERSION, Status::FILE_RETRIEVED_OK, req.name_len,
        req.filename, fileSize);

        Serializer serObj;
        std::vector<uint8_t> serialized = serObj.serializeFullFileResponse(fullFileRes);
        boost::asio::write(sock, boost::asio::buffer(serialized.data(), serialized.size()));

        char payloadBuffer[BUFFER_SIZE];
        while (true) {
            file.read(payloadBuffer, sizeof(payloadBuffer)); // reads 4096 bytes from file to the buffer
            std::streamsize size = file.gcount();
            if (size <= 0) {
                break;
            }
            boost::asio::write(sock, boost::asio::buffer(payloadBuffer, file.gcount()));
        }
        file.close();
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cout << e.what() << std::endl;
    }
}

void ClientHandler::handle_delete_req(boost::asio::ip::tcp::socket& sock, const FileOpsRequest& req) {
    try {
        const std::filesystem::path targetUserPath = BASE_DIR / std::to_string(req.uid);
        if (!folder_exists(targetUserPath.string())) {
            // send no user exists response
            Response res{};
            res.version = VERSION;
            res.status = Status::NO_FILES_IN_SERVER_ERR;
            boost::asio::write(sock, boost::asio::buffer(&res, sizeof(res)));

            std::cout << "Response number " <<  static_cast<int>(res.status) << " was sent to client" << std::endl << std::endl;
        }
        const std::filesystem::path fileTargetPath = safe_path(targetUserPath, req.filename);
        if (remove(fileTargetPath)) {
            std::cout << "File " << req.filename << " was deleted" << std::endl;
            PartialFileResponse partRes(VERSION, Status::FILE_OPS_OK, req.name_len, req.filename);
            Serializer serObj;
            std::vector<uint8_t> serialized = serObj.serializePartialResponse(partRes);
            boost::asio::write(sock, boost::asio::buffer(serialized.data(), serialized.size()));

            std::cout << "Response number " <<  static_cast<int>(partRes.status) << " with file '" <<  partRes.filename << "' was sent to client" << std::endl << std::endl;
        }
        else {
            PartialFileResponse partRes(VERSION, Status::FILE_NOT_EXISTS_ERR, req.name_len, req.filename);
            Serializer serObj;
            std::vector<uint8_t> serialized = serObj.serializePartialResponse(partRes);
            boost::asio::write(sock, boost::asio::buffer(serialized.data(), serialized.size()));

            std::cout << "Response number " <<  static_cast<int>(partRes.status) << " with file '" <<  partRes.filename << "' was sent to client" << std::endl << std::endl;
        }
    }
    catch (GeneralException& e) {
        std::cout << e.what() << std::endl;
        e.sendGeneralErrorResponse(std::move(sock));
    }
}

void ClientHandler::handle_save_req(boost::asio::ip::tcp::socket& sock, const SaveFileRequest& req) {
    try {
        const std::filesystem::path userTargetPath = BASE_DIR / std::to_string(req.uid);
        create_directories(userTargetPath);

        const std::filesystem::path filePath = safe_path(userTargetPath, req.filename);
        save_payload_to_file(sock, filePath, req.size);

        PartialFileResponse partRes(VERSION, Status::FILE_OPS_OK, req.name_len, req.filename);
        Serializer serObj;
        std::vector<uint8_t> serialized = serObj.serializePartialResponse(partRes);
        boost::asio::write(sock, boost::asio::buffer(serialized.data(), serialized.size()));

        std::cout << "Response number " <<  static_cast<int>(partRes.status) << " with file '" <<  partRes.filename << "' was sent to client" << std::endl << std::endl;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        Response res{};
        res.version = VERSION;
        res.status = Status::NO_FILES_IN_SERVER_ERR;
        boost::asio::write(sock, boost::asio::buffer(&res, sizeof(res)));

        std::cout << "Response number " <<  static_cast<int>(res.status) << " was sent to client" << std::endl << std::endl;
    }
    catch (GeneralException& e) {
        std::cerr << e.what() << std::endl;
        e.sendGeneralErrorResponse(std::move(sock));
    }
}



void ClientHandler::handle_list_req(boost::asio::ip::tcp::socket& sock, const Request& req) {

    TempFile tmpFilename(generateFilename());

    std::filesystem::create_directories(tmpFilename.path.parent_path());
    try {
        ListRequest listReq(req);
        std::filesystem::path clientPath = safe_path(BASE_DIR, std::to_string(listReq.uid));

        const std::vector<std::string> defaultFiles = {
            "backup.info",
            "server.info",
            "Request.py",
            "FilesReader.py",
            "client.py"
        };

        std::ofstream outNewFile(tmpFilename.path);
        if (!outNewFile.is_open()) {
            throw GeneralException("A general error occurred when trying to create a file");
            // general error, throw exception
        }

        for (const auto& entry : std::filesystem::directory_iterator(clientPath)) {
            std::string currFileName = entry.path().filename().string();
            if (find(defaultFiles.begin(), defaultFiles.end(),
                    currFileName) == defaultFiles.end()) {
                outNewFile << currFileName << std::endl;
            }
        }

        outNewFile.flush();
        outNewFile.close();

        std::ifstream inNewFile(tmpFilename.path, std::ios::binary); // open file in binary mode
        if (!inNewFile.is_open()) {
            // general error, throw exception
            throw GeneralException("Couldn't open file for reading");
        }

        inNewFile.seekg(0, std::ios::end);
        uint32_t tmpFileSize =  inNewFile.tellg();
        inNewFile.seekg(0, std::ios::beg);

        uint16_t pathToNewTmpFileLength = static_cast<uint16_t>(tmpFilename.path.string().length());
        FullFileResponse fullFileRes(VERSION, Status::LIST_FILES_OK, pathToNewTmpFileLength,
            tmpFilename.path.string(), tmpFileSize);

        Serializer serObj;
        std::vector<uint8_t> serialized = serObj.serializeFullFileResponse(fullFileRes);
        boost::asio::write(sock, boost::asio::buffer(serialized.data(), serialized.size()));

        char payloadBuffer[BUFFER_SIZE];
        while (true) {
            inNewFile.read(payloadBuffer, sizeof(payloadBuffer));
            std::streamsize size = inNewFile.gcount();
            if (size <= 0) {
                break;
            }
            boost::asio::write(sock, boost::asio::buffer(payloadBuffer, inNewFile.gcount()));
        }
        inNewFile.close();

        std::cout << "Response number " <<  static_cast<int>(fullFileRes.status) << " with file '" <<  fullFileRes.filename << "' was sent to client" << std::endl << std::endl;

    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        Response res{};
        res.version = VERSION;
        res.status = Status::NO_FILES_IN_SERVER_ERR;
        boost::asio::write(sock, boost::asio::buffer(&res, sizeof(res)));

        std::cout << "Response number " <<  static_cast<int>(res.status) << " was sent to client" << std::endl << std::endl;
    }

    catch (GeneralException& e) {
        std::cerr << e.what() << std::endl;
        e.sendGeneralErrorResponse(std::move(sock));
    }

    // clientHandlerSock.close();
}

void ClientHandler::save_payload_to_file(boost::asio::ip::tcp::socket& sock, const std::filesystem::path &filePath, const uint32_t fileSize) {
    std::ofstream file(filePath, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        throw GeneralException("Couldn't open file for writing");
    }
    if (fileSize == 0) {
        throw GeneralException("The payload is empty");
    }
    std::vector<char> buffer(4096);
    uint32_t remainingSize = fileSize;

    while (remainingSize > 0) {
        std::size_t bytesToRead = std::min<uint32_t>(buffer.size(), remainingSize);
        std::size_t bytesRead = boost::asio::read(sock, boost::asio::buffer(buffer.data(), bytesToRead));

        file.write(buffer.data(), bytesRead);
        remainingSize -= bytesRead;
    }
    file.close();
}

std::string ClientHandler::generateFilename() {
    constexpr size_t length = 32;
    std::random_device rand;
    std::mt19937 gen(rand());
    std::uniform_int_distribution<> distrib(0, 61);

    std::string result;
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

std::filesystem::path ClientHandler::safe_path(const std::filesystem::path& baseDir, const std::string& filename) {
    const std::filesystem::path targetFilePath = weakly_canonical(baseDir / filename);
    // weakly_canonical resolves folderPath/filename into absolute path (without . and ..)
    if (targetFilePath.native().find(BASE_DIR.native()) != 0) {
        // if the base path of targetFilePath is different from the base path of folderPath
        // meaning there that an attempt for path traversal
        throw GeneralException("Invalid filename (path traversal attempt)");
    }
    return targetFilePath;
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