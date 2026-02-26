//
// Created by yash2 on 2/26/2026.
//

#include "Connection.h"

bool Connection::send_bytes(const std::unique_ptr<const std::byte> data,const size_t len){
    return true;
}
bool Connection::recv_bytes(const std::unique_ptr<const std::byte> buffer, const size_t len){
    return true;
}
bool Connection::send_handshake(const Handshake& hs){
    return true;
}
bool Connection::recv_handshake(Handshake& hs){
    return true;
}
bool Connection::send_message(const Message& msg){
    return true;
}
bool Connection::recv_message(Message& msg){
    return true;
}
void Connection::close(){
}