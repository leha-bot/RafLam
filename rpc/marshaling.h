#pragma once

#include <typeinfo>
#include <tuple>

#include "protocol/redis.h"

//extern template class SocketWriter<boost::asio::ip::tcp::socket>;
//extern template class SocketWriter<boost::asio::local::stream_protocol::socket>;


//typedef std::shared_ptr<SocketWriter<boost::asio::ip::tcp::socket>> reader_type;
//typedef std::shared_ptr<Reader> Reader_ptr;

template<class... Args >
class Marshaling {
    // за основу возьмем http://www.jsonrpc.org/specification
    typedef std::shared_ptr<Writer> Writer_ptr;

public:
    explicit Marshaling(Writer_ptr writer, std::string& f, Args&&... args ) :
            func_name_(f),
            tuple_(args...),
            args_count_(sizeof...(args)),
            w_(writer)
    {}

    explicit Marshaling(Writer_ptr writer, std::string& f) :
            func_name_(f),
            tuple_(),
            args_count_(0),
            w_(writer)
    {}

    template <typename T>
    void convert(T& val, RedisValue& rval);

    void compose_request(int id);
    void compose_response(int id);

protected:
    std::string func_name_;
    std::tuple<Args...> tuple_;
    size_t args_count_;
    Writer_ptr w_;

    void setVersion_();
    void setType_(int type);
    void setMethod_();
    void setParams_();
    void setId_(int id);
    void setResponse_();
};


template<class... Args>
template<typename T>
void Marshaling<Args...>::convert(T& val, RedisValue& rval) {
    try {
        if (typeid(val) == typeid(int)) {
            rval = val;

        } else if (typeid(val) == typeid(std::string)) {
            rval = val;

//        } else if (typeid(val) == typeid(std::vector<T>)) { TODO: разобраться с шаблонным вектором
//            boost::get<std::vector<RedisValue>>(rval).resize(val.size());
//            for (uint64_t i = 0; i < val.size(); ++i) {
//                convert<T>(val[i], boost::get<std::vector<RedisValue>>(rval)[i]);
//            }

        } else {
            throw std::runtime_error("RPC-marshaling: Unsupported type");
        }
    } catch (const std::exception& e) {
        rval = RedisError(e.what());
        throw;
    }
}

template<class... Args>
void Marshaling<Args...>::setVersion_() {
    RedisValue val;

    std::string protocol_version = "redis-rpc";

    convert<std::string>(protocol_version, val);
    protocol::WriteRedisValue(w_, val);

    int version = 1;

    convert<int>(version, val);
    protocol::WriteRedisValue(w_, val);
}

template<class... Args>
void Marshaling<Args...>::setType_(int type) {
    std::string t;

    switch (type) {
        case 1: {
            t = "method";
            break;
        } case 2 : {
            t = "result";
            break;
        } default: {
            throw std::runtime_error("Marshaling: Unsupported type");
        }
    }

    RedisValue val;

    convert<std::string>(t, val);
    protocol::WriteRedisValue(w_, val);
}

template<class... Args>
void Marshaling<Args...>::setMethod_() {
    RedisValue val;

    convert<std::string>(func_name_, val);
    protocol::WriteRedisValue(w_, val);
}


template<class... Args>
void Marshaling<Args...>::setParams_() {
    RedisValue val;
//    for (size_t i = 0; i < args_count_; ++i) {
//        int elem = std::get<i>(tuple_);
//        convert<int>(elem, val);
//        protocol::WriteRedisValue(w_, val);
//    }
    int elem = std::get<0>(tuple_);
    convert<int>(elem, val);
    protocol::WriteRedisValue(w_, val);
}

template<class... Args>
void Marshaling<Args...>::setId_(int id) {
    RedisValue val;

    std::string protocol_version = "id";

    convert<std::string>(protocol_version, val);
    protocol::WriteRedisValue(w_, val);

    convert<int>(id, val);
    protocol::WriteRedisValue(w_, val);
}

template<class... Args>
void Marshaling<Args...>::setResponse_() {
    RedisValue val;

    convert<std::string>(func_name_, val);
    protocol::WriteRedisValue(w_, val);
}

template<class... Args>
void Marshaling<Args...>::compose_request(int id) {
    setVersion_();
    setId_(id);
    setType_(1);
    setMethod_();
    setParams_();
}

template<class... Args>
void Marshaling<Args...>::compose_response(int id) {
    setVersion_();
    setId_(id);
    setType_(2);
    setResponse_();
}

//    extern template class SocketReader<boost::asio::ip::tcp::socket>;
//
//    typedef SocketReader<boost::asio::ip::tcp::socket> reader_type;
