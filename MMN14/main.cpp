#include <boost/asio.hpp>
#include <string>
#include <thread>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <algorithm>

using namespace std;
using namespace boost::asio;
using namespace std::filesystem;

#pragma pack(push, 1)
struct Request {
    uint32_t uid;
    uint8_t op;
    uint8_t version;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Response {
    uint8_t status;
    uint8_t version;
};
#pragma pack(pop)

void clear(char* message, int length) {
    for (int i=0 ; i < length ; i++) {
        message[i] = '\0';
    }
}

void session(ip::tcp::socket sock) {
    try {
        while (true) {

            Request req{};
            read(sock, buffer(&req, sizeof(req)));

            cout << "uid, op, version: " << req.uid << ", " << static_cast<int>(req.op) << ", " << static_cast<int>(req.version) << endl;

            const string clientPath = to_string(req.uid);

            if (req.op == 202) {
                const vector<string> defaultFiles = {
                    "backup.info",
                    "server.info",
                    "Request.py",
                    "FilesReader.py",
                    "client.py"
                };
                vector<string> userFiles;

                for (const auto& entry : directory_iterator(clientPath)) {
                    string currFileName = entry.path().filename().string();
                    if (is_regular_file(entry.status()) &&
                        find(defaultFiles.begin(),
                            defaultFiles.end(),
                            currFileName) == defaultFiles.end()) {
                        userFiles.push_back(currFileName);
                    }
                }
                if (userFiles.empty()) {
                    Response newRes = {};
                    newRes.status = 1002;
                    newRes.version = 1;
                }
            }
        }
    }
    catch (const boost::system::system_error& e) {
        if (e.code() == error::eof) {
            cout << "Client disconnected (EOF)" << endl;
        }
        else {
            cerr << "Exception in thread: " << e.what() << endl;
        }
    }
    catch (const filesystem_error& e) {
        cout << "No such user" << endl; // error 1002
    }
}

void server(io_context& context, unsigned short int port) {
    ip::tcp::acceptor a(context, ip::tcp::endpoint(ip::tcp::v4(), port));
    while (true) {
        session(a.accept());
    }
}

int main() {
    const string address = "127.0.0.1";
    const string port = "1234";
    try {
        io_context context;
        server(context, stoi(port));
    }
    catch (exception& e) {
        cerr << "Exception in thread: " << e.what() << endl;
    }
}