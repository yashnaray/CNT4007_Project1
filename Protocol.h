//
// Created by yash2 on 2/21/2026.
//

#ifndef CNT4007_PROJECT1_PROTOCOL_H
#define CNT4007_PROJECT1_PROTOCOL_H
#include <cassert>
#include <memory>
#include <vector>

enum MessageType : uint8_t {
    CHOKE,
    UNCHOKE,
    INTERESTED,
    NOT_INTERESTED,
    HAVE,
    BITFIELD,
    REQUEST,
    PIECE
};

struct Handshake {
    const uint8_t message[18] = {'P','2','P','F','I','L','E','S','H','A','R','I','N','G','P','R','O','J'};
    uint8_t zero_bits[10] = {0,0,0,0,0,0,0,0,0,0};
    uint32_t peer_ID{};
};

struct Have {
    uint32_t piece_index;
};

struct Bitfield {
    std::vector<uint8_t> bitfield;
    
    explicit Bitfield(const size_t num_pieces) {
        const size_t num_bytes = (num_pieces + 7) / 8;
        bitfield.resize(num_bytes, 0);
    }
    
    void set_piece(uint32_t piece_index) {
        size_t byte_index = piece_index / 8;
        uint8_t bit_index = 7 - (piece_index % 8);
        bitfield[byte_index] |= (1 << bit_index);
    }
    
    bool has_piece(uint32_t piece_index) const {
        const size_t byte_index = piece_index / 8;
        const uint8_t bit_index = 7 - (piece_index % 8);
        return (bitfield[byte_index] & (1 << bit_index)) != 0;
    }
};
struct Piece {
    uint32_t piece_index;
    std::vector<uint8_t> content;
};

struct Choke {};
struct Unchoke {};
struct Interested {};
struct NotInterested {};
struct Request {
    uint32_t piece_index;
};

struct Message {
    MessageType message_type{};
    uint32_t message_len{};
    std::vector<uint8_t> payload;
};

class Protocol {

};

Message create_choke_message();
Message create_unchoke_message();
Message create_interested_message();
Message create_not_interested_message();
Message create_have_message(uint32_t piece_index);
Message create_bitfield_message(const Bitfield& bitfield);
Message create_request_message(uint32_t piece_index);
Message create_piece_message(uint32_t piece_index, const std::vector<uint8_t>& content);
Message create_simple_message(MessageType type);

#endif 