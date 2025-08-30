//
// Created by Shaked Pollak on 8/30/2025.
//

#ifndef GENERALEXCEPTIONS_H
#define GENERALEXCEPTIONS_H

#include <exception>
#include <string>

using namespace std;

class GeneralException : public std::exception {
private:
    string message;

public:
    GeneralException(const string& msg = "A general error has occurred") : message(msg) {}

    const char* what() const noexcept override {
        // override what()
        return message.c_str();
    }
};

#endif //GENERALEXCEPTIONS_H
