#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include <boost/asio.hpp>

#include "../configs.h"

using boost::asio::ip::tcp;


class Writer {
public:
    explicit Writer(uint64_t buffer_size) : buffer_(buffer_size) {}

    void write_string(const std::string& s);
    void write_raw(const char* s, size_t len);
    void write_int(int64_t i);
    void write_char(char c);
    void write_crlf(); // write "\r\n"

    virtual void flush() = 0;

protected:
    std::vector<char> buffer_;
    uint64_t wpos_ = 0;
};


class StringWriter : public Writer {
public:
    explicit StringWriter(uint64_t buffer_size = 1024) : Writer(buffer_size) {}

    std::string result;

    virtual void flush() override;
};


template <class SocketType>
class SocketWriter : public Writer {
private:
    std::shared_ptr<SocketType> socket_;
public:
    explicit SocketWriter(std::shared_ptr<SocketType> socket, uint64_t buffer_size = 1024) :
            socket_(socket),
            Writer(buffer_size)
    {}

    virtual void flush() override;
};