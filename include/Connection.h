#ifndef PROGRAM_SOCKET_INTERFACE_H
#define PROGRAM_SOCKET_INTERFACE_H

#include <cstdint>
#include <cstddef>
#include "Protocol.h"

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

class Connection {
    int socket_fd;
    uint32_t peer_id = 0;
    std::mutex send_mutex;
    
public:
    explicit Connection(int fd) : socket_fd(fd) {}
    ~Connection() { close(); }
    
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&& other) noexcept : socket_fd(other.socket_fd), peer_id(other.peer_id) {
        other.socket_fd = -1;
    }
    
    bool send_bytes(const void* data, size_t len);
    bool recv_bytes(void* buffer, size_t len);
    bool send_handshake(const Handshake& hs);
    bool recv_handshake(Handshake& hs);
    bool send_message(const Message& msg);
    bool recv_message(Message& msg);
    void close();
    
    void set_peer_id(uint32_t id) { peer_id = id; }
    uint32_t get_peer_id() const { return peer_id; }
    int get_socket() const { return socket_fd; }
};

#endif