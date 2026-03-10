#ifndef CNT4007_PROJECT1_LOGGER_H
#define CNT4007_PROJECT1_LOGGER_H

#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <set>
#include <cstdint>

class Logger {
    std::ofstream log_file;
    uint32_t peer_id;
    
    std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    
public:
    explicit Logger(uint32_t id) : peer_id(id) {
        std::string filename = "log_peer_" + std::to_string(id) + ".log";
        log_file.open(filename, std::ios::out | std::ios::app);
    }
    
    ~Logger() { if (log_file.is_open()) log_file.close(); }
    
    void log_tcp_connection(uint32_t peer1, uint32_t peer2) {
        log_tcp_connection_to(peer1, peer2);
    }

    void log_tcp_connection_to(uint32_t peer1, uint32_t peer2) {
        log_file << "[" << get_timestamp() << "]: Peer " << peer1 
                 << " makes a connection to Peer " << peer2 << ".\n";
        log_file.flush();
    }

    void log_tcp_connection_from(uint32_t peer1, uint32_t peer2) {
        log_file << "[" << get_timestamp() << "]: Peer " << peer1 
                 << " is connected from Peer " << peer2 << ".\n";
        log_file.flush();
    }
    
    void log_change_preferred_neighbors(uint32_t peer_id, const std::set<uint32_t>& neighbors) {
        log_file << "[" << get_timestamp() << "]: Peer " << peer_id 
                 << " has the preferred neighbors ";
        bool first = true;
        for (uint32_t n : neighbors) {
            if (!first) log_file << ", ";
            log_file << n;
            first = false;
        }
        log_file << ".\n";
        log_file.flush();
    }

    void log_change_optimistically_unchoked_neighbor(uint32_t peer_id, uint32_t opt_neighbor) {
        log_file << "[" << get_timestamp() << "]: Peer " << peer_id 
                 << " has the optimistically unchoked neighbor " << opt_neighbor << ".\n";
        log_file.flush();
    }
    
    void log_unchoking(uint32_t peer1, uint32_t peer2) {
        log_file << "[" << get_timestamp() << "]: Peer " << peer1 
                 << " is unchoked by " << peer2 << ".\n";
        log_file.flush();
    }
    
    void log_choking(uint32_t peer1, uint32_t peer2) {
        log_file << "[" << get_timestamp() << "]: Peer " << peer1 
                 << " is choked by " << peer2 << ".\n";
        log_file.flush();
    }
    
    void log_received_have(uint32_t peer1, uint32_t peer2, uint32_t piece_idx) {
        log_file << "[" << get_timestamp() << "]: Peer " << peer1 
                 << " received the 'have' message from " << peer2 
                 << " for the piece " << piece_idx << ".\n";
        log_file.flush();
    }
    
    void log_received_interested(uint32_t peer1, uint32_t peer2) {
        log_file << "[" << get_timestamp() << "]: Peer " << peer1 
                 << " received the 'interested' message from " << peer2 << ".\n";
        log_file.flush();
    }
    
    void log_received_not_interested(uint32_t peer1, uint32_t peer2) {
        log_file << "[" << get_timestamp() << "]: Peer " << peer1 
                 << " received the 'not interested' message from " << peer2 << ".\n";
        log_file.flush();
    }
    
    void log_downloading_piece(uint32_t peer1, uint32_t piece_idx, uint32_t peer2, size_t num_pieces) {
        log_file << "[" << get_timestamp() << "]: Peer " << peer1 
                 << " has downloaded the piece " << piece_idx << " from " << peer2 << ". "
                 << "Now the number of pieces it has is " << num_pieces << ".\n";
        log_file.flush();
    }
    
    void log_completion(uint32_t peer_id) {
        log_file << "[" << get_timestamp() << "]: Peer " << peer_id 
                 << " has downloaded the complete file.\n";
        log_file.flush();
    }
};

#endif