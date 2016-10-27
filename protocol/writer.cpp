#include <iostream>
#include "writer.h"


void Writer::write_int(int64_t i) {
    char buff[128];
    int len = snprintf(buff, sizeof(buff), "%ld", i);
    if (len >= 0) {
        write_raw(buff, len);
    } else {
        throw std::bad_array_new_length(); //TODO: разобраться какое искючение кидать
    }
}

void Writer::write_raw(const char* s, uint64_t len) {
    if (len > MAX_LENGHT_ROWSTRING) {
        throw std::invalid_argument("Too large raw_string");
    }

    for(int i = 0; i < len; ) {
        while(i < len && wpos_ != buffer_.size()) {
            buffer_[wpos_++] = s[i++];
        }

        if (wpos_ == buffer_.size())
            flush();
    }
}

void Writer::write_char(char c) {
    buffer_[wpos_++] = c;

    if (wpos_ == buffer_.size())
        flush();
}

void Writer::write_crlf() {
    write_char('\r');
    write_char('\n');
}

void Writer::write_string(const std::string &s) {
    if (s.size() > MAX_LENGHT_STRING) {
        throw std::invalid_argument("Too large row string");
    }
    write_raw(s.data(), s.size());
}


void StringWriter::flush() {
    result.append(buffer_.begin(), buffer_.begin() + wpos_);
    wpos_ = 0;
}


template <class SocketType>
void SocketWriter<SocketType>::flush() {
    boost::asio::write(*socket_, boost::asio::buffer(buffer_, wpos_));
    wpos_ = 0;
}

template class SocketWriter<boost::asio::ip::tcp::socket>;
template class SocketWriter<boost::asio::local::stream_protocol::socket>;