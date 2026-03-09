#include "Peer.h"
#include "Client.h"
#include <iostream>

void Peer::handle_connection(Connection& conn) {
    Handshake hs;
    if (!conn.recv_handshake(hs)) return;
    
    conn.set_peer_id(hs.peer_ID);
    add_neighbor(hs.peer_ID);
    logger.log_tcp_connection(peer_id, hs.peer_ID);
    
    Handshake my_hs;
    my_hs.peer_ID = peer_id;
    conn.send_handshake(my_hs);
    conn.send_message(create_bitfield_message(my_bitfield));
    
    while (running) {
        Message msg;
        if (!conn.recv_message(msg)) break;
        
        switch (msg.message_type) {
            case CHOKE: {
                std::lock_guard lock(neighbors_mutex);
                if (auto it = neighbors.find(conn.get_peer_id()); it != neighbors.end()) {
                    it->second.is_choked = true;
                }
                logger.log_choking(peer_id, conn.get_peer_id());
                break;
            }
                
            case UNCHOKE: {
                {
                    std::lock_guard lock(neighbors_mutex);
                    if (auto it = neighbors.find(conn.get_peer_id()); it != neighbors.end()) {
                        it->second.is_choked = false;
                    }
                }
                logger.log_unchoking(peer_id, conn.get_peer_id());
                auto piece = select_piece_to_request(conn.get_peer_id());
                if (piece != UINT32_MAX) {
                    conn.send_message(create_request_message(piece));
                }
                break;
            }
                
            case INTERESTED: {
                std::lock_guard lock(neighbors_mutex);
                if (auto it = neighbors.find(conn.get_peer_id()); it != neighbors.end()) {
                    it->second.is_interested = true;
                }
                logger.log_received_interested(peer_id, conn.get_peer_id());
                break;
            }
                
            case NOT_INTERESTED: {
                std::lock_guard lock(neighbors_mutex);
                if (auto it = neighbors.find(conn.get_peer_id()); it != neighbors.end()) {
                    it->second.is_interested = false;
                }
                logger.log_received_not_interested(peer_id, conn.get_peer_id());
                break;
            }
                
            case HAVE: {
                uint32_t piece_idx;
                std::memcpy(&piece_idx, msg.payload.data(), 4);
                piece_idx = ntohl(piece_idx);
                update_neighbor_piece(conn.get_peer_id(), piece_idx);
                logger.log_received_have(peer_id, conn.get_peer_id(), piece_idx);
                
                if (is_interested_in(conn.get_peer_id())) {
                    conn.send_message(create_interested_message());
                } else {
                    conn.send_message(create_not_interested_message());
                }
                break;
            }
                
            case BITFIELD: {
                Bitfield bf(num_pieces);
                bf.bitfield = msg.payload;
                update_neighbor_bitfield(conn.get_peer_id(), bf);
                
                if (is_interested_in(conn.get_peer_id())) {
                    conn.send_message(create_interested_message());
                }
                break;
            }
                
            case REQUEST: {
                uint32_t piece_idx;
                std::memcpy(&piece_idx, msg.payload.data(), 4);
                piece_idx = ntohl(piece_idx);
                
                bool should_send = false;
                {
                    std::lock_guard lock(neighbors_mutex);
                    if (auto it = neighbors.find(conn.get_peer_id()); 
                        it != neighbors.end() && !it->second.is_choked && my_bitfield.has_piece(piece_idx)) {
                        should_send = true;
                    }
                }
                
                if (should_send) {
                    std::vector<uint8_t> piece_data;
                    if (file_manager.read_piece(piece_idx, piece_data)) {
                        conn.send_message(create_piece_message(piece_idx, piece_data));
                    }
                }
                break;
            }
                
            case PIECE: {
                uint32_t piece_idx;
                std::memcpy(&piece_idx, msg.payload.data(), 4);
                piece_idx = ntohl(piece_idx);
                std::vector<uint8_t> content(msg.payload.begin() + 4, msg.payload.end());
                
                file_manager.write_piece(piece_idx, content);
                mark_piece_received(piece_idx, conn.get_peer_id(), content.size());
                
                if (!has_complete_file()) {
                    auto next_piece = select_piece_to_request(conn.get_peer_id());
                    if (next_piece != UINT32_MAX) {
                        conn.send_message(create_request_message(next_piece));
                    }
                }
                break;
            }
        }
    }
}

void Peer::start_server(uint16_t port) {
    Server server;
    if (!server.bind_and_listen(port)) return;
    
    while (running) {
        auto conn = server.accept_connection();
        if (conn) {
            connection_threads.emplace_back(&Peer::handle_connection, this, std::ref(*conn));
        }
    }
}

void Peer::connect_to_peers(const std::vector<PeerInfo>& peers) {
    for (const auto& p : peers) {
        auto conn = Client::connect_to_peer(p.hostname, p.port);
        if (conn) {
            connection_threads.emplace_back(&Peer::handle_connection, this, std::ref(*conn));
        }
    }
}