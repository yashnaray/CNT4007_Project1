#ifndef CNT4007_PROJECT1_PEER_H
#define CNT4007_PROJECT1_PEER_H

#include "Protocol.h"
#include <map>
#include <set>
#include <random>
#include <algorithm>
#include <ranges>
#include <concepts>
#include <pthread.h>
#include "Server.h"
struct PeerInfo {
    uint32_t peer_id;
    std::string hostname;
    uint16_t port;
    bool has_file;
};

struct NeighborState {
    uint32_t peer_id;
    Bitfield bitfield;
    bool is_choked = true;
    bool is_interested = false;
    bool am_interested = false;
    size_t download_rate = 0;
    size_t bytes_downloaded = 0;
    
    NeighborState() : peer_id(0), bitfield(0) {}
    
    NeighborState(const uint32_t id, const size_t num_pieces)
        : peer_id(id), bitfield(num_pieces) {}
};

class Peer {
    uint32_t peer_id;
    Bitfield my_bitfield;
    size_t num_pieces;
    size_t k_preferred;
    std::map<uint32_t, NeighborState> neighbors;
    std::set<uint32_t> preferred_neighbors;
    uint32_t optimistic_unchoke_neighbor = 0;
    std::set<uint32_t> requested_pieces;
    std::mt19937 rng;
    std::vector<std::thread> connection_threads;

    void handle_connection(Connection& conn) {
    }
    
    void start_server(uint16_t port) {
        Server server;
        server.bind_and_listen(port);
        
        while (Server.isRunning()) {
            Connection* conn = server.accept_connection();
            connection_threads.emplace_back(&Peer::handle_connection, this, conn);
        }
    }
    
    void connect_to_peers(const std::vector<PeerInfo>& peers) {
        for (auto& p : peers) {
            Connection* conn = connect_to_peer(p.hostname, p.port);
            connection_threads.emplace_back(&Peer::handle_connection, this, conn);
        }
    }
};

public:
    Peer(const uint32_t id, size_t num_pieces, const size_t k_pref, const bool has_complete_file)
        : peer_id(id), my_bitfield(num_pieces), num_pieces(num_pieces), 
          k_preferred(k_pref), rng(std::random_device{}()) {
        if (has_complete_file) {
            for (auto i : std::views::iota(0u, num_pieces)) {
                my_bitfield.set_piece(i);
            }
        }
    }
    
    void add_neighbor(uint32_t neighbor_id) {
        neighbors.emplace(neighbor_id, NeighborState(neighbor_id, num_pieces));
    }
    
    void update_neighbor_bitfield(const uint32_t neighbor_id, const Bitfield& bitfield) {
        if (const auto it = neighbors.find(neighbor_id); it != neighbors.end()) {
            it->second.bitfield = bitfield;
        }
    }
    
    void update_neighbor_piece(const uint32_t neighbor_id, const uint32_t piece_index) {
        if (const auto it = neighbors.find(neighbor_id); it != neighbors.end()) {
            it->second.bitfield.set_piece(piece_index);
        }
    }
    
    bool is_interested_in(const uint32_t neighbor_id) {
        const auto it = neighbors.find(neighbor_id);
        if (it == neighbors.end()) return false;
        
        return std::ranges::any_of(std::views::iota(0u, num_pieces), [&](auto i) {
            return it->second.bitfield.has_piece(i) && !my_bitfield.has_piece(i);
        });
    }
    
    void select_preferred_neighbors() {
        auto interested = neighbors | std::views::filter([](auto& p) { return p.second.am_interested; });
        std::vector<std::pair<size_t, uint32_t>> rates;
        std::ranges::transform(interested, std::back_inserter(rates), 
            [](auto& p) { return std::pair{p.second.download_rate, p.first}; });
        
        has_complete_file() ? std::ranges::shuffle(rates, rng) : std::ranges::sort(rates, std::greater{});
        
        std::set<uint32_t> new_preferred;
        std::ranges::transform(rates | std::views::take(k_preferred), 
            std::inserter(new_preferred, new_preferred.end()), 
            [](auto& p) { return p.second; });
        
        for (auto id : new_preferred) {
            if (!preferred_neighbors.contains(id)) neighbors[id].is_choked = false;
        }
        
        for (auto id : preferred_neighbors) {
            if (!new_preferred.contains(id) && id != optimistic_unchoke_neighbor) {
                neighbors[id].is_choked = true;
            }
        }
        
        preferred_neighbors = std::move(new_preferred);
        std::ranges::for_each(neighbors, [](auto& p) { p.second.download_rate = p.second.bytes_downloaded = 0; });
    }
    
    void select_optimistic_unchoke() {
        auto candidates = neighbors 
            | std::views::filter([&](auto& p) { 
                return p.second.is_choked && p.second.am_interested && !preferred_neighbors.contains(p.first); 
              })
            | std::views::keys;
        
        std::vector<uint32_t> vec(candidates.begin(), candidates.end());
        if (!vec.empty()) {
            optimistic_unchoke_neighbor = vec[std::uniform_int_distribution<size_t>(0, vec.size() - 1)(rng)];
            neighbors[optimistic_unchoke_neighbor].is_choked = false;
        }
    }
    
    uint32_t select_piece_to_request(uint32_t neighbor_id) {
        auto it = neighbors.find(neighbor_id);
        if (it == neighbors.end()) return UINT32_MAX;
        
        auto available = std::views::iota(0u, num_pieces)
            | std::views::filter([&](auto i) {
                return it->second.bitfield.has_piece(i) && !my_bitfield.has_piece(i) && !requested_pieces.contains(i);
              });
        
        std::vector<uint32_t> vec;
        for (auto piece : available) {
            vec.push_back(piece);
        }
        if (vec.empty()) return UINT32_MAX;
        
        uint32_t piece = vec[std::uniform_int_distribution<size_t>(0, vec.size() - 1)(rng)];
        requested_pieces.insert(piece);
        return piece;
    }
    
    void mark_piece_received(uint32_t piece_index, uint32_t from_peer, size_t piece_size) {
        my_bitfield.set_piece(piece_index);
        requested_pieces.erase(piece_index);
        if (auto it = neighbors.find(from_peer); it != neighbors.end()) {
            it->second.bytes_downloaded += piece_size;
            it->second.download_rate += piece_size;
        }
    }
    
    bool has_complete_file() const {
        return std::ranges::all_of(std::views::iota(0u, num_pieces), 
            [&](auto i) { return my_bitfield.has_piece(i); });
    }
    
    const Bitfield& get_bitfield() const { return my_bitfield; }
    const std::set<uint32_t>& get_preferred_neighbors() const { return preferred_neighbors; }
    uint32_t get_optimistic_unchoke() const { return optimistic_unchoke_neighbor; }
};

#endif 
