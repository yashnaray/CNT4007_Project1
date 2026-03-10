//
// Created by yash2 on 2/21/2026.
//

#ifndef CNT4007_PROJECT1_CONFIG_READER_H
#define CNT4007_PROJECT1_CONFIG_READER_H


#include <string>
#include <vector>
#include <cstdint>
#include "Peer.h"

struct CommonConfig {
    int num_preferred_neighbors;
    int unchoking_interval;
    int optimistic_unchoking_interval;
    std::string file_name;
    size_t file_size;
    size_t piece_size;
};

class Config_reader {
public:
    static CommonConfig read_common_cfg(const std::string& filepath);
    static std::vector<PeerInfo> read_peer_info(const std::string& filepath);
};

#endif //CNT4007_PROJECT1_CONFIG_READER_H