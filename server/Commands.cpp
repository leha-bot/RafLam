#include "Commands.h"

/*
Set::Set(Storage * storage) {
    if (storage->getPtrOfStorage() != nullptr) {
        storage_ = storage->getPtrOfStorage();
    } else {
        throw std::invalid_argument("Pointer of storage is NULL!");
    }
}

const std::string Set::name() const {
    return name_;
}

RedisValue Set::exec(RedisValue& arg) {
    storage_->operator[](boost::get<RedisBulkString>(boost::get<std::vector<RedisValue>> (arg)[1]).data) =
            boost::get<RedisBulkString>(boost::get<std::vector<RedisValue>> (arg)[2]).data;
    return RedisBulkString("OK");
}


Get::Get(Storage * storage) {
    if (storage->getPtrOfStorage() != nullptr) {
        storage_ = storage->getPtrOfStorage();
    } else {
        throw std::invalid_argument("Pointer of storage is NULL!");
    }
}

const std::string Get::name() const {
    return name_;
}

RedisValue Get::exec(RedisValue& arg) {
    return RedisBulkString(storage_->at(boost::get<RedisBulkString>(boost::get<std::vector<RedisValue>> (arg)[1]).data));
}
*/

Cmd::Cmd(command name, Storage * storage) :
        name_(name)
        , storage_(storage->getPtrOfStorage())
{}

command Cmd::name() {
    return name_;
}


Set::Set(Storage * storage) : Cmd(SET, storage) {}

RedisValue Set::exec(RedisValue& arg) {
    storage_->operator[](boost::get<RedisBulkString>(boost::get<std::vector<RedisValue>> (arg)[1]).data) =
            boost::get<RedisBulkString>(boost::get<std::vector<RedisValue>> (arg)[2]).data;
    return std::string("OK");
}


Get::Get(Storage * storage) : Cmd(GET, storage) {}

RedisValue Get::exec(RedisValue& arg) {
    try {
        return RedisBulkString(
                storage_->at(boost::get<RedisBulkString>(boost::get<std::vector<RedisValue>>(arg)[1]).data));
    } catch (std::out_of_range& e) {
        std::cout << "Can't find element\n";
        return RedisNull();
    }
}