#include <boost/asio.hpp>
#include <string>
#include <thread>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include "Network/Server.h"

using namespace std;
using namespace boost::asio;
using namespace std::filesystem;

// const path BASE_DIR = "C:\\backupsrv";

int main() {
    const string address = "127.0.0.1";
    const string port = "5000";
    try {
        // if (exists(BASE_DIR))
        //     cout << "exists1" << endl;
        io_context context;
        Server server(context, 5000);
        server.run();
    }
    catch (exception& e) {
        cerr << "Exception in thread: " << e.what() << endl;
    }
}