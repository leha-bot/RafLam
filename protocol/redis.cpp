#include "redis.h"


void WriteRedisValue(Writer_ptr w, const RedisValue& value) {
    if (value.which() == REDIS_INT) {
        w->write_char(':');
        w->write_int(boost::get<int64_t>(value));
        w->write_crlf();

    } else if (value.which() == REDIS_STRING) {
        w->write_char('+');
        w->write_string(boost::get<std::string>(value));
        w->write_crlf();

    } else if (value.which() == REDIS_ERROR) {
        w->write_char('-');
        w->write_string(boost::get<RedisError>(value).msg);
        w->write_crlf();

    } else if (value.which() == REDIS_NULL) {
        w->write_char('$');
        w->write_int(static_cast<int64_t>(-1));
        w->write_crlf();

    } else if (value.which() == REDIS_ARRAY) {
        w->write_char('*');
        if (boost::get<std::vector<RedisValue>>(value).size() > MAX_LENGHT_ARRAY) {
            throw std::invalid_argument("Too large array");
        } else {
            w->write_int(boost::get<std::vector<RedisValue>>(value).size());
            w->write_crlf();
            for (auto elem : boost::get<std::vector<RedisValue>>(value)) {
                WriteRedisValue(w, elem);
            }
        }

    } else if (value.which() == REDIS_BULK) {
        w->write_char('$');
        w->write_int(boost::get<RedisBulkString>(value).data.size());
        w->write_crlf();
        w->write_raw(boost::get<RedisBulkString>(value).data.c_str(),
                     boost::get<RedisBulkString>(value).data.size());
        w->write_crlf();

    } else {
        throw std::runtime_error("Redis-protocol: Unsupported type");
    }
}


void ReadRedisValue(Reader_ptr r, RedisValue& value) {
    switch(r->read_char()) {
        case ':': {
            value = r->read_int();
            break;

        } case '+': {
            value = r->read_line();
            break;

        } case '-': {
            value = RedisError(r->read_line());
            break;

        } case '$': {
            int64_t len = r->read_int();
            if (len == -1) {
                value = RedisNull();
            } else {
                value = RedisBulkString(r->read_raw(len));
            }
            break;

        } case '*': {
            int64_t len = r->read_int();
            value = std::vector<RedisValue>();

            boost::get<std::vector<RedisValue>>(value).resize(len);

            for (int64_t i = 0; i < len; ++i) {
                ReadRedisValue(r, boost::get<std::vector<RedisValue>>(value)[i]);
            }
            break;

        } default:
            throw std::runtime_error("Redis-protocol: Invalid redis value");
    }
}
