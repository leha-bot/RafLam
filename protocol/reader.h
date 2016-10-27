#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

#include <boost/asio.hpp>

#include "../configs.h"

using boost::asio::ip::tcp;


class Reader {
public:
    explicit Reader(size_t buffer_size) : buffer_(buffer_size) {}

    char read_char(); // read single char
    std::string read_line(); // read until "\r\n" and skip it
    std::string read_raw(size_t len); // read string with fixed size and skip "\r\n" at the end
    int64_t read_int(); // read integer and skip "\r\n"

protected:
    std::vector<char> buffer_;
    size_t end_ = 0, rpos_ = 0;

    virtual void read_more() = 0;
};


class StringReader : public Reader {
public:
    explicit StringReader(size_t buffer_size = 1024) : Reader(buffer_size) {}

    std::string input;

    virtual void read_more() override;
};


template <class SocketType>
class SocketReader : public Reader {
private:
    std::shared_ptr<SocketType> socket_;
public:
    explicit SocketReader(std::shared_ptr<SocketType> socket, uint64_t buffer_size = 1024) :
        socket_(socket),
        Reader(buffer_size)
    {}

    virtual void read_more() override;
};
