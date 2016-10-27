#pragma once

#include "../protocol/redis.h"
#include "../configs.h"
#include "Storage.h"


class Cmd {
protected:
    command name_;
    std::unordered_map<std::string, std::string> * storage_;
public:
    Cmd(command name, Storage * storage);
    command name();
    virtual RedisValue exec(RedisValue& args) = 0;
};


class Set : protected Cmd {
public:
    Set(Storage * table);
    using Cmd::name;
    RedisValue exec(RedisValue& args);
};


class Get : protected Cmd {
public:
    Get(Storage * table);
    using Cmd::name;
    RedisValue exec(RedisValue& args);
};