//
// Created by yash2 on 2/26/2026.
//
#include "Connection.h"
#ifndef PROGRAM_SERVER_H
#define PROGRAM_SERVER_H


class Server {
   int listen_fd;
    
public:
    bool bind_and_listen(uint16_t port, int backlog = 10);
    Connection* accept_connection();
    void close();
};


#endif //PROGRAM_SERVER_H