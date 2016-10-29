#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <memory>

#include <boost/variant.hpp>

#include "reader.h"
#include "writer.h"


struct RedisError {
    explicit RedisError(const std::string& msg) : msg(msg) {}

    std::string msg;
};
struct RedisBulkString {
    explicit RedisBulkString(const std::string& data) : data(data) {}

    std::string data;
};

struct RedisNull {
    explicit RedisNull() {}
};

typedef boost::make_recursive_variant<
    RedisNull,
    int64_t,
    std::string,
    RedisError,
    RedisBulkString,
    std::vector<boost::recursive_variant_>
>::type RedisValue;

enum RedisType {
    REDIS_NULL,
    REDIS_INT,
    REDIS_STRING,
    REDIS_ERROR,
    REDIS_BULK,
    REDIS_ARRAY
};

namespace protocol {
    typedef std::shared_ptr<Reader> Reader_ptr;
    typedef std::shared_ptr<Writer> Writer_ptr;

    void WriteRedisValue(Writer_ptr w, const RedisValue &value);

    void ReadRedisValue(Reader_ptr r, RedisValue &value);
}
