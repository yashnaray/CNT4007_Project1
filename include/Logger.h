#ifndef CNT4007_PROJECT1_LOGGER_H
#define CNT4007_PROJECT1_LOGGER_H

#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

class Logger {
    std::ofstream log_file;
    uint32_t peer_id;
    
    std::string get_timestamp() {}
    
public:
    explicit Logger(uint32_t id) : peer_id(id) {
    }
    
    ~Logger() { if (log_file.is_open()) log_file.close(); }
    
    void log_tcp_connection(uint32_t peer1, uint32_t peer2) {
    }
    
    void log_change_preferred_neighbors(uint32_t peer_id, const std::set<uint32_t>& neighbors) {
       
    }
    
    void log_unchoking(uint32_t peer1, uint32_t peer2) {
       
    }
    
    void log_choking(uint32_t peer1, uint32_t peer2) {
        
    }
    
    void log_received_have(uint32_t peer1, uint32_t peer2, uint32_t piece_idx) {
       
    }
    
    void log_received_interested(uint32_t peer1, uint32_t peer2) {
        
    }
    
    void log_received_not_interested(uint32_t peer1, uint32_t peer2) {
       
    }
    
    void log_downloading_piece(uint32_t peer1, uint32_t piece_idx, uint32_t peer2, size_t num_pieces) {
        
    }
    
    void log_completion(uint32_t peer_id) {
        
    }
};

#endif
