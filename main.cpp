#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <csignal>
#include <memory>
#include "include/Protocol.h"
#include "include/Peer.h"
#include "include/Config_reader.h"

std::unique_ptr<std::jthread> g_server_thread;
std::unique_ptr<std::jthread> g_unchoke_thread;
std::unique_ptr<std::jthread> g_optimistic_thread;

void signal_handler(int sig) {
    if (g_server_thread) g_server_thread->request_stop();
    if (g_unchoke_thread) g_unchoke_thread->request_stop();
    if (g_optimistic_thread) g_optimistic_thread->request_stop();
}

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

    g_server_thread = std::make_unique<std::jthread>([&peer, &my_info](std::stop_token st) {
        peer.start_server(my_info.port, std::move(st));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    peer.connect_to_peers(previous_peers);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    g_unchoke_thread = std::make_unique<std::jthread>([&peer, &common_cfg](std::stop_token st) {
        peer.select_preferred_neighbors();
        while (!st.stop_requested()) {
            for (int i = 0; i < common_cfg.unchoking_interval && !st.stop_requested(); ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            if (!st.stop_requested()) {
                peer.select_preferred_neighbors();
            }
        }
    });

    g_optimistic_thread = std::make_unique<std::jthread>([&peer, &common_cfg](std::stop_token st) {
        for (int i = 0; i < common_cfg.optimistic_unchoking_interval && !st.stop_requested(); ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        while (!st.stop_requested()) {
            peer.select_optimistic_unchoke();
            for (int i = 0; i < common_cfg.optimistic_unchoking_interval && !st.stop_requested(); ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    });

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    while (!peer.all_peers_complete()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "All peers have the complete file. Shutting down." << std::endl;
    peer.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (g_server_thread && g_server_thread->joinable()) {
        g_server_thread->request_stop();
        g_server_thread->join();
    }
    if (g_unchoke_thread && g_unchoke_thread->joinable()) {
        g_unchoke_thread->request_stop();
        g_unchoke_thread->join();
    }
    if (g_optimistic_thread && g_optimistic_thread->joinable()) {
        g_optimistic_thread->request_stop();
        g_optimistic_thread->join();
    }

    return 0;
}