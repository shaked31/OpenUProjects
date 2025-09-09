//
// Created by Shaked Pollak on 8/28/2025.
//

#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H
#define VERSION 1


#include <boost/asio.hpp>
#include "Protocols/ProtocolEnums.h"
#include "Protocols/ClientRequests.h"
#include <iostream>
#include <filesystem>
#include <fstream>

class ClientHandler {
public:
    void start_session(boost::asio::ip::tcp::socket sock);

private:
    void handle_list_req(boost::asio::ip::tcp::socket& sock, const Request& req);
    void handle_retrieve_req(boost::asio::ip::tcp::socket& sock, const FileOpsRequest& req);
    void handle_delete_req(boost::asio::ip::tcp::socket& sock, const FileOpsRequest& req);
    void handle_save_req(boost::asio::ip::tcp::socket& sock, const SaveFileRequest &req);

    bool folder_exists(const std::string& path);
    std::filesystem::path safe_path(const std::filesystem::path& baseDir, const std::string& filename);
    std::string generateFilename();
    void save_payload_to_file(boost::asio::ip::tcp::socket& sock, const std::filesystem::path& filePath, const uint32_t fileSize);

    struct TempFile {
        std::filesystem::path path;
        TempFile(const std::string& filename) : path(std::filesystem::temp_directory_path() / "backupsrv" / (filename + ".txt")) {}
        ~TempFile() {
            std::error_code ec;
            std::filesystem::remove(path.string().c_str(), ec);
            if (ec) {
                std::cout << ec.message() << std::endl;
                // throw std::filesystem::filesystem_error("Couldn't remove temp file", ec);
            }
        }
    };
};


#endif //CLIENTHANDLER_H
