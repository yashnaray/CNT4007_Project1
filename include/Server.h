#pragma once
#ifndef PROGRAM_SERVER_H
#define PROGRAM_SERVER_H

#include "Connection.h"
#include <memory>

class Server {
    int listen_fd = -1;
    bool is_running = false;
    
public:
    Server() = default;
    ~Server() { close(); }
    
    bool bind_and_listen(uint16_t port, int backlog = 10);
    std::shared_ptr<Connection> accept_connection();
    void close();
    bool is_listening() const { return is_running; }
};

#endif