#pragma once

#include <deque>
#include <functional>
#include <string>
#include "Listener.h"
#include "Storage.h"
#include "Commands.h"
#include "../protocol/redis.h"


class Server {
protected:
    Listener listener_;
public:
    explicit Server(int port = 6379, unsigned int maxCli = 10);
    virtual void serve() = 0;
};

class TestServer : protected Server {
public:
    using Server::Server;
    unsigned int countConn_ = 0;
    std::unique_ptr<Socket> out_;
    virtual void serve() override;
};

class ProdServer : protected Server {
public:
    using Server::Server;
    virtual void serve() override;
};

