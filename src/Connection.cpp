#include "Connection.h"
#include <cstring>

#ifdef _WIN32
static bool winsock_initialized = false;
void init_winsock() {
    if (!winsock_initialized) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        winsock_initialized = true;
    }
}
#endif

bool Connection::send_bytes(const void* data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        auto result = ::send(socket_fd, static_cast<const char*>(data) + sent, len - sent, 0);
        if (result <= 0) return false;
        sent += result;
    }
    return true;
}

bool Connection::recv_bytes(void* buffer, size_t len) {
    size_t received = 0;
    while (received < len) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(socket_fd, &read_fds);
        
        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int ready = select(socket_fd + 1, &read_fds, nullptr, nullptr, &timeout);
        if (ready < 0) return false;
        if (ready == 0) {
            continue;
        }
        
        auto result = ::recv(socket_fd, static_cast<char*>(buffer) + received, len - received, 0);
        if (result <= 0) return false;
        received += result;
    }
    return true;
}

bool Connection::send_handshake(const Handshake& hs) {
    std::lock_guard<std::mutex> lock(send_mutex);
    uint8_t buffer[32];
    std::memcpy(buffer, hs.message, 18);
    std::memcpy(buffer + 18, hs.zero_bits, 10);
    uint32_t peer_id_net = htonl(hs.peer_ID);
    std::memcpy(buffer + 28, &peer_id_net, 4);
    return send_bytes(buffer, 32);
}

bool Connection::recv_handshake(Handshake& hs) {
    uint8_t buffer[32];
    if (!recv_bytes(buffer, 32)) return false;
    std::memcpy(const_cast<uint8_t*>(hs.message), buffer, 18);
    std::memcpy(hs.zero_bits, buffer + 18, 10);
    uint32_t peer_id_net;
    std::memcpy(&peer_id_net, buffer + 28, 4);
    hs.peer_ID = ntohl(peer_id_net);
    return true;
}

bool Connection::send_message(const Message& msg) {
    std::lock_guard<std::mutex> lock(send_mutex);
    uint32_t len_net = htonl(msg.message_len);
    if (!send_bytes(&len_net, 4)) return false;
    if (!send_bytes(&msg.message_type, 1)) return false;
    if (!msg.payload.empty() && !send_bytes(msg.payload.data(), msg.payload.size())) return false;
    return true;
}

bool Connection::recv_message(Message& msg) {
    uint32_t len_net;
    if (!recv_bytes(&len_net, 4)) return false;
    msg.message_len = ntohl(len_net);
    if (!recv_bytes(&msg.message_type, 1)) return false;
    if (msg.message_len > 1) {
        msg.payload.resize(msg.message_len - 1);
        if (!recv_bytes(msg.payload.data(), msg.message_len - 1)) return false;
    }
    return true;
}

void Connection::close() {
    if (socket_fd >= 0) {
#ifdef _WIN32
        closesocket(socket_fd);
#else
        ::close(socket_fd);
#endif
        socket_fd = -1;
    }
}