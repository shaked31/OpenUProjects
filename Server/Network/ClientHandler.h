//
// Created by Shaked Pollak on 8/28/2025.
//

#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H
#define VERSION 1


#include <boost/asio.hpp>
#include "Protocols/Protocol.h"
#include "Protocols/ClientRequests.h"
#include <iostream>
#include <filesystem>
#include <fstream>



using boost::asio::ip::tcp;
using namespace std;
using namespace filesystem;

class ClientHandler {
public:
    void start_session(tcp::socket sock);

private:
    void handle_list_req(tcp::socket sock, const Request& req);
    void handle_retrieve_req(tcp::socket sock, const FileOpsRequest& req);
    void handle_delete_req(tcp::socket sock, const FileOpsRequest& req);
    void handle_save_req(tcp::socket sock, const SaveFileRequest &req);

    bool folder_exists(const std::string& path);
    path safe_path(const path& baseDir, const string& filename);
    string generateFilename();
    void save_payload_to_file(tcp::socket sock, path& filePath, uint32_t fileSize);
};



#endif //CLIENTHANDLER_H
