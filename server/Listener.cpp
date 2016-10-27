#include "Listener.h"


Listener::Listener(int port, unsigned int maxCli) {
    if (port < 0 || port > 65535) {
        throw std::invalid_argument("LISTENER: Bad id of port!");
    } else {
        port_ = port;
    }

    if (maxCli < 0) {
        throw std::invalid_argument("LISTENEAR: bad amount of clients!");
    } else {
        maxCountClient_ = maxCli;
    }

    socket_();
    setOpt_();
    bind_();
    listen_();
}

void Listener::socket_() {
    if ((socListenerDisc_ = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("LISTENER: Can't open listener socket!");
    }
    std::cout << "listener: start\n";
}

void Listener::setOpt_() {
    int opt = 1;
    setsockopt(socListenerDisc_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void Listener::bind_() {
    struct sockaddr_in ba;
    ba.sin_family = AF_INET;
    ba.sin_port = htons(port_);
    ba.sin_addr.s_addr = INADDR_ANY;
    if (bind(socListenerDisc_, (struct sockaddr*) &ba, sizeof(ba)) < 0) {
        throw std::runtime_error("LISTENER: Can't bind of listener socket!");
    }
}

void Listener::listen_() {
    if (listen(socListenerDisc_, maxCountClient_) < 0) {
        throw std::runtime_error("LISTENER: Can't listen to socket!");
    }
}

std::unique_ptr<Socket> Listener::acceptCli() {
    struct sockaddr_in aa;
    socklen_t slen = sizeof(aa);

    std::unique_ptr<Socket> newClient(new Socket(accept(socListenerDisc_, (struct sockaddr *) &aa, &slen)));
    newClient->options_ = aa;
    std::cout << "listener: get a client\n";
    return newClient;
}

Listener::~Listener() {
    close(socListenerDisc_);
}