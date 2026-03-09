#ifndef PROGRAM_CLIENT_H
#define PROGRAM_CLIENT_H

#include "Connection.h"
#include <string>
#include <memory>

class Client {
public:
    static std::unique_ptr<Connection> connect_to_peer(const std::string& hostname, uint16_t port);
};

#endif