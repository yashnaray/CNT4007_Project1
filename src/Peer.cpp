#include "Peer.h"
#include "Client.h"
#include <iostream>
#include <memory>
#include <cstring>
#include <stop_token>

void Peer::handle_connection(std::shared_ptr<Connection> conn, bool is_client, std::stop_token st) {
    Handshake my_hs;
    my_hs.peer_ID = peer_id;
    if (!conn->send_handshake(my_hs)) return;

    Handshake hs;
    if (!conn->recv_handshake(hs)) return;
    
    conn->set_peer_id(hs.peer_ID);
    add_neighbor(hs.peer_ID);
    register_connection(hs.peer_ID, conn);
    
    if (is_client) {
        logger.log_tcp_connection_to(peer_id, hs.peer_ID);
    } else {
        logger.log_tcp_connection_from(peer_id, hs.peer_ID);
    }
    
    conn->send_message(create_bitfield_message(my_bitfield));
    
    while (!st.stop_requested()) {
        Message msg;
        if (!conn->recv_message(msg)) break;
        
        switch (msg.message_type) {
            case CHOKE: {
                std::vector<std::pair<uint32_t, std::shared_ptr<Connection>>> to_notify;
                {
                    std::lock_guard lock(neighbors_mutex);
                    if (auto it = neighbors.find(conn->get_peer_id()); it != neighbors.end()) {
                        it->second.am_choked = true;
                    }
                    std::erase_if(requested_pieces, [&](const auto& entry) {
                        return entry.second == conn->get_peer_id();
                    });
                    for (auto& [nid, nconn] : connections) {
                        if (nid != conn->get_peer_id()) {
                            to_notify.emplace_back(nid, nconn);
                        }
                    }
                }
                logger.log_choking(peer_id, conn->get_peer_id());
                for (auto& [nid, nconn] : to_notify) {
                    send_interest_update(nid, nconn);
                }
                break;
            }
                
            case UNCHOKE: {
                {
                    std::lock_guard lock(neighbors_mutex);
                    if (auto it = neighbors.find(conn->get_peer_id()); it != neighbors.end()) {
                        it->second.am_choked = false;
                    }
                }
                logger.log_unchoking(peer_id, conn->get_peer_id());
                auto piece = select_piece_to_request(conn->get_peer_id());
                if (piece == UINT32_MAX && !has_complete_file()) {
                    {
                        std::lock_guard lock(neighbors_mutex);
                        requested_pieces.clear();
                    }
                    piece = select_piece_to_request(conn->get_peer_id());
                }
                if (piece != UINT32_MAX) {
                    conn->send_message(create_request_message(piece));
                }
                break;
            }

            case INTERESTED: {
                std::lock_guard lock(neighbors_mutex);
                if (auto it = neighbors.find(conn->get_peer_id()); it != neighbors.end()) {
                    it->second.is_interested = true;
                }
                logger.log_received_interested(peer_id, conn->get_peer_id());
                break;
            }
                
            case NOT_INTERESTED: {
                std::lock_guard lock(neighbors_mutex);
                if (auto it = neighbors.find(conn->get_peer_id()); it != neighbors.end()) {
                    it->second.is_interested = false;
                }
                logger.log_received_not_interested(peer_id, conn->get_peer_id());
                break;
            }
                
            case HAVE: {
                uint32_t piece_idx;
                std::memcpy(&piece_idx, msg.payload.data(), 4);
                piece_idx = ntohl(piece_idx);
                update_neighbor_piece(conn->get_peer_id(), piece_idx);
                logger.log_received_have(peer_id, conn->get_peer_id(), piece_idx);
                send_interest_update(conn->get_peer_id(), conn);
                break;
            }
                
            case BITFIELD: {
                Bitfield bf(num_pieces);
                bf.bitfield = msg.payload;
                update_neighbor_bitfield(conn->get_peer_id(), bf);
                bool interested = is_interested_in(conn->get_peer_id());
                {
                    std::lock_guard lock(neighbors_mutex);
                    if (auto it = neighbors.find(conn->get_peer_id()); it != neighbors.end()) {
                        it->second.am_interested = interested;
                    }
                }
                conn->send_message(interested 
                    ? create_interested_message() 
                    : create_not_interested_message());
                break;
            }

            case REQUEST: {
                uint32_t piece_idx;
                std::memcpy(&piece_idx, msg.payload.data(), 4);
                piece_idx = ntohl(piece_idx);
                
                bool should_send = false;
                {
                    std::lock_guard lock(neighbors_mutex);
                    if (auto it = neighbors.find(conn->get_peer_id()); 
                        it != neighbors.end() && !it->second.is_choked && my_bitfield.has_piece(piece_idx)) {
                        should_send = true;
                    }
                }
                
                if (should_send) {
                    std::vector<uint8_t> piece_data;
                    bool read_ok;
                    {
                        std::lock_guard file_lock(file_mutex);
                        read_ok = file_manager.read_piece(piece_idx, piece_data);
                    }
                    if (read_ok) {
                        conn->send_message(create_piece_message(piece_idx, piece_data));
                    }
                }
                break;
            }
                
            case PIECE: {
                uint32_t piece_idx;
                std::memcpy(&piece_idx, msg.payload.data(), 4);
                piece_idx = ntohl(piece_idx);
                std::vector<uint8_t> content(msg.payload.begin() + 4, msg.payload.end());
                
                {
                    std::lock_guard file_lock(file_mutex);
                    file_manager.write_piece(piece_idx, content);
                }
                mark_piece_received(piece_idx, conn->get_peer_id(), content.size());
                
                if (!has_complete_file()) {
                    auto next_piece = select_piece_to_request(conn->get_peer_id());
                    if (next_piece != UINT32_MAX) {
                        conn->send_message(create_request_message(next_piece));
                    }
                }
                break;
            }
        }
    }
    
    uint32_t closed_peer_id = conn->get_peer_id();
    {
        std::lock_guard lock(neighbors_mutex);
        if (has_complete_file_unlocked()) {
            if (auto it = neighbors.find(closed_peer_id); it != neighbors.end()) {
                for (unsigned i = 0; i < num_pieces; ++i) {
                    it->second.bitfield.set_piece(i);
                }
            }
        }
        connections.erase(closed_peer_id);
        preferred_neighbors.erase(closed_peer_id);
        if (optimistic_unchoke_neighbor == closed_peer_id) {
            optimistic_unchoke_neighbor = 0;
        }
        std::erase_if(requested_pieces, [&](const auto& entry) {
            return entry.second == closed_peer_id;
        });
    }
}

void Peer::start_server(uint16_t port, std::stop_token st) {
    Server server;
    if (!server.bind_and_listen(port)) {
        std::cerr << "Peer " << peer_id << ": Failed to bind to port " << port << std::endl;
        return;
    }
    std::cout << "Peer " << peer_id << ": Server listening on port " << port << std::endl;
    
    while (!st.stop_requested()) {
        auto conn = server.accept_connection(st);
        if (!conn) {
            if (st.stop_requested()) break;
            continue;
        }
        std::cout << "Peer " << peer_id << ": Accepted connection" << std::endl;
        {
            std::lock_guard lock(neighbors_mutex);
            owned_connections.push_back(conn);
        }
        auto st_copy = std::make_shared<std::stop_token>(st);
        connection_threads.emplace_back([this, conn, st_copy]() {
            handle_connection(conn, false, *st_copy);
        });
    }
    server.close();
}

void Peer::connect_to_peers(const std::vector<PeerInfo>& peers) {
    for (const auto& p : peers) {
        std::cout << "Peer " << peer_id << ": Attempting to connect to peer " << p.peer_id 
                  << " at " << p.hostname << ":" << p.port << std::endl;
        auto conn = Client::connect_to_peer(p.hostname, p.port);
        if (conn) {
            std::cout << "Peer " << peer_id << ": Successfully connected to peer " << p.peer_id << std::endl;
            {
                std::lock_guard lock(neighbors_mutex);
                owned_connections.push_back(conn);
            }
            connection_threads.emplace_back([this, conn]() {
            std::stop_token st;
            handle_connection(conn, true, st);
        });
        } else {
            std::cerr << "Peer " << peer_id << ": Failed to connect to peer " << p.peer_id << std::endl;
        }
    }
}