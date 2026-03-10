//
// Created by yash2 on 2/21/2026.
//

#include "Config_reader.h"
#include <fstream>
#include <sstream>
#include <iostream>

CommonConfig Config_reader::read_common_cfg(const std::string& filepath) {
    CommonConfig config{};
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
        return config;
    }

    std::string line, key;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        iss >> key;
        if (key == "NumberOfPreferredNeighbors") iss >> config.num_preferred_neighbors;
        else if (key == "UnchokingInterval") iss >> config.unchoking_interval;
        else if (key == "OptimisticUnchokingInterval") iss >> config.optimistic_unchoking_interval;
        else if (key == "FileName") iss >> config.file_name;
        else if (key == "FileSize") iss >> config.file_size;
        else if (key == "PieceSize") iss >> config.piece_size;
    }
    return config;
}

std::vector<PeerInfo> Config_reader::read_peer_info(const std::string& filepath) {
    std::vector<PeerInfo> peers;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
        return peers;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        PeerInfo peer;
        int has_file;
        if (iss >> peer.peer_id >> peer.hostname >> peer.port >> has_file) {
            peer.has_file = (has_file == 1);
            peers.push_back(peer);
        }
    }
    return peers;
}