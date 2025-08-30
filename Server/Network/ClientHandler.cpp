//
// Created by Shaked Pollak on 8/28/2025.
//

#include "ClientHandler.h"

#include <random>

#include "../Utils/Serializer.h"
#include "Protocols/ServerResponses.h"
#include "Protocols/Protocol.h"
#include "../Exceptions/GeneralException.h"

using namespace std;
using namespace std::filesystem;
using namespace boost::asio;

const path BASE_DIR = "C://backupsrv//";


ClientHandler::ClientHandler(tcp::socket sock) :
    clientHandlerSock(std::move(sock)) {
    start_session();
} // init clientHandlerSock to sock


void ClientHandler::start_session() {

    Request req{};
    read(clientHandlerSock, buffer(&req, sizeof(req)));
    cout << int(req.op) << endl;;
    switch (req.op) {
        case OpCode::LIST_FILES:
            handle_list_req(req);
        case OpCode::RETRIEVE_FILE:
            break;
            // handle_retreive_req();
        case OpCode::DELETE_FILE:
            break;
            // handle_delete_req();
        case OpCode::SAVE_FILE:
            break;
            // handle_save_req();
    }
}


void ClientHandler::handle_list_req(const Request req) {
    const path tmpFilename = temp_directory_path() / "backupsrv" / (generateFilename() + ".txt");
    create_directories(tmpFilename.parent_path());
    try {
        ListRequest listReq(req);
        path clientPath = BASE_DIR / to_string(listReq.uid);

        // create_directories(clientPath); // tmp command
        // ofstream f1(clientPath / "hello.txt");// tmp command
        // ofstream f2(clientPath / "Tcp.docx");// tmp command

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
            throw GeneralException();
        }

        inNewFile.seekg(0, ios::end);
        uint32_t tmpFileSize =  inNewFile.tellg();
        inNewFile.seekg(0, ios::beg);

        uint16_t pathToNewTmpFileLength = static_cast<uint16_t>(tmpFilename.string().length());
        FullFileResponse fullFileRes(VERSION, Status::LIST_FILES_OK, pathToNewTmpFileLength,
            tmpFilename.string(), tmpFileSize);

        Serializer serObj;

        vector<uint8_t> serialized = serObj.serializeResponse(fullFileRes);


        write(clientHandlerSock, buffer(serialized.data(), serialized.size()));


        char payloadBuffer[4096];
        while (true) {
            inNewFile.read(payloadBuffer, sizeof(payloadBuffer));
            streamsize size = inNewFile.gcount();
            if (size <= 0) {
                break;
            }
            write(clientHandlerSock, buffer(payloadBuffer, inNewFile.gcount()));
        }
        inNewFile.close();
    }
    catch (const filesystem_error& e) {
        cerr << e.what() << endl;
        Response res{};
        res.version = VERSION;
        res.status = Status::NO_FILES_IN_SERVER_ERR;
        write(clientHandlerSock, buffer(&res, sizeof(res)));
    }

    catch (const GeneralException& e) {
        cerr << e.what() << endl;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
    }

    // clientHandlerSock.close();
}

string ClientHandler::generateFilename() {
    size_t length = 32;
    random_device rand;
    mt19937 gen(rand());
    uniform_int_distribution<> distrib(0, 61);

    string result;
    result.reserve(length);

    for (size_t i=0 ; i<length ; i++) {
        int num = distrib(gen);
        if (num < 10) {
            // 0-9  char
             result += '0' + num;
        }
        else if (num < 36) {
            // A-Z char
            result += 'A' + num-10;
        }
        else {
            // a-z char
            result += 'a' + num-36;
        }
    }

    return result;
}