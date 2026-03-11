#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "include/Protocol.h"
#include "include/Peer.h"
#include "include/Config_reader.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./peerProcess <peer_id>" << std::endl;
        return 1;
    }
    
    uint32_t my_peer_id = std::stoi(argv[1]);

    CommonConfig common_cfg = Config_reader::read_common_cfg("Common.cfg");
    std::vector<PeerInfo> peer_list = Config_reader::read_peer_info("PeerInfo.cfg");

    PeerInfo my_info;
    std::vector<PeerInfo> previous_peers;
    bool found_myself = false;

    for (const auto& peer : peer_list) {
        if (peer.peer_id == my_peer_id) {
            my_info = peer;
            found_myself = true;
        } else if (!found_myself) {
            previous_peers.push_back(peer);
        }
    }

    if (!found_myself) {
        std::cerr << "Error: Peer ID " << my_peer_id << " not found in PeerInfo.cfg" << std::endl;
        return 1;
    }

    std::cout << "Starting Peer " << my_peer_id << " on port " << my_info.port << "..." << std::endl;

    size_t num_pieces = 0;
    if (common_cfg.piece_size > 0) {
        num_pieces = (common_cfg.file_size + common_cfg.piece_size - 1) / common_cfg.piece_size;
    }

    Peer peer(my_peer_id, 
              num_pieces, 
              common_cfg.num_preferred_neighbors, 
              my_info.has_file, 
              common_cfg.file_name, 
              common_cfg.file_size, 
              common_cfg.piece_size);

    std::thread server_thread(&Peer::start_server, &peer, my_info.port);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    peer.connect_to_peers(previous_peers);

    // Give connections time to establish and exchange handshakes/bitfields
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread unchoke_thread([&peer, &common_cfg]() {
        peer.select_preferred_neighbors();  // Initial selection
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(common_cfg.unchoking_interval));
            peer.select_preferred_neighbors();
        }
    });

    std::thread optimistic_thread([&peer, &common_cfg]() {
        std::this_thread::sleep_for(std::chrono::seconds(common_cfg.optimistic_unchoking_interval));
        while (true) {
            peer.select_optimistic_unchoke();
            std::this_thread::sleep_for(std::chrono::seconds(common_cfg.optimistic_unchoking_interval));
        }
    });

    server_thread.join();
    unchoke_thread.join();
    optimistic_thread.join();

    return 0;
}