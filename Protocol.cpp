//
// Created by yash2 on 2/21/2026.
//

#include "Protocol.h"

// Create a Have message
Message create_have_message(uint32_t piece_index) {
    Message msg;
    msg.message_type = HAVE;
    msg.message_len = 5;
    msg.payload.resize(4);
    std::memcpy(msg.payload.data(), &piece_index, 4);
    return msg;
}

Message create_bitfield_message(const Bitfield& bitfield) {
    Message msg;
    msg.message_type = BITFIELD;
    msg.message_len = 1 + bitfield.bitfield.size();
    msg.payload = bitfield.bitfield;
    return msg;
}

// Create a Piece message
Message create_piece_message(uint32_t piece_index, const std::vector<uint8_t>& content) {
    Message msg;
    msg.message_type = PIECE;
    msg.message_len = 1 + 4 + content.size();
    msg.payload.resize(4 + content.size());
    std::memcpy(msg.payload.data(), &piece_index, 4);
    std::memcpy(msg.payload.data() + 4, content.data(), content.size());
    return msg;
}

Message create_request_message(uint32_t piece_index) {
    Message msg;
    msg.message_type = REQUEST;
    msg.message_len = 5;
    msg.payload.resize(4);
    std::memcpy(msg.payload.data(), &piece_index, 4);
    return msg;
}

Message create_simple_message(MessageType type) {
    Message msg;
    msg.message_type = type;
    msg.message_len = 1;
    return msg;
}