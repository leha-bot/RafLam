#pragma once

#include <unordered_map>
#include "../protocol/redis.h"

class Storage {
private:
    std::unordered_map<std::string, std::string> storage_;
public:
    std::unordered_map<std::string, std::string> * getPtrOfStorage();
};