//
// Created by yash2 on 2/26/2026.
//
#include <cstdint>
#include <cstddef>
#include "Protocol.h"
#ifndef PROGRAM_SOCKET_INTERFACE_H
#define PROGRAM_SOCKET_INTERFACE_H


class Connection {
    const int socket_fd;
    uint32_t peer_id;
public:
    Connection(const int fd) : socket_fd(fd) {}
    bool send_bytes(const std::unique_ptr<const std::byte> data, const size_t len);
    bool recv_bytes(const std::unique_ptr<const std::byte> buffer, const size_t len);
    bool send_handshake(const Handshake& hs);
    bool recv_handshake(Handshake& hs);
    bool send_message(const Message& msg);
    bool recv_message(Message& msg);
    void close();
};


#endif