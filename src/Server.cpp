#include "Server.h"
#include <cstring>

#ifdef _WIN32
extern void init_winsock();
#endif

bool Server::bind_and_listen(uint16_t port, int backlog) {
#ifdef _WIN32
    init_winsock();
#endif
    
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) return false;
    
    int opt = 1;
#ifdef _WIN32
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close();
        return false;
    }
    
    if (listen(listen_fd, backlog) < 0) {
        close();
        return false;
    }
    
    is_running = true;
    return true;
}

std::shared_ptr<Connection> Server::accept_connection(std::stop_token st) {
    while (!st.stop_requested()) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(listen_fd, &read_fds);
        
        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int ready = select(listen_fd + 1, &read_fds, nullptr, nullptr, &timeout);
        if (ready < 0) return nullptr;
        if (ready == 0) continue;
        
        if (st.stop_requested()) return nullptr;
        
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
        if (client_fd >= 0) {
            return std::make_shared<Connection>(client_fd);
        }
    }
    return nullptr;
}

void Server::close() {
    if (listen_fd >= 0) {
#ifdef _WIN32
        closesocket(listen_fd);
#else
        ::close(listen_fd);
#endif
        listen_fd = -1;
        is_running = false;
    }
}