#include "Client.h"
#include <cstring>

#ifdef _WIN32
extern void init_winsock();
#endif

std::unique_ptr<Connection> Client::connect_to_peer(const std::string& hostname, uint16_t port) {
#ifdef _WIN32
    init_winsock();
#endif
    
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) return nullptr;
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
#ifdef _WIN32
    addr.sin_addr.s_addr = inet_addr(hostname.c_str());
#else
    if (inet_pton(AF_INET, hostname.c_str(), &addr.sin_addr) <= 0) {
        ::close(sock_fd);
        return nullptr;
    }
#endif
    
    if (connect(sock_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
#ifdef _WIN32
        closesocket(sock_fd);
#else
        ::close(sock_fd);
#endif
        return nullptr;
    }
    
    return std::make_unique<Connection>(sock_fd);
}