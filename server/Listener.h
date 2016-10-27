#pragma once

#include <memory>
#include <iostream>

#include "Socket.h"

class Listener {
private:
    int socListenerDisc_ = -1;
    int maxCountClient_ = -1;
    int port_ = -1;
protected:
    void socket_();
    void setOpt_();
    void bind_();
    void listen_();
public:
    Listener(int port, unsigned int maxCli);
    std::unique_ptr<Socket> acceptCli();
    ~Listener();
};

