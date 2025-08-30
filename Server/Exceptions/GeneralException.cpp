//
// Created by Shaked Pollak on 8/30/2025.
//

#include "GeneralExceptions.h"

const char* what() const noexcept override {
    // override what()
    return message.c_str();
}
