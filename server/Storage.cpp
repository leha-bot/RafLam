#include "Storage.h"

std::unordered_map<std::string, std::string> * Storage::getPtrOfStorage() {
    return &storage_;
}