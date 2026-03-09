#ifndef CNT4007_PROJECT1_FILEMANAGER_H
#define CNT4007_PROJECT1_FILEMANAGER_H

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include "common.h"
class FileManager {
    std::string filename;
    size_t file_size;
    size_t piece_size;
    size_t num_pieces;
    
public:
    FileManager(const std::string& fname, size_t fsize, size_t psize): filename(fname), file_size(fsize), piece_size(psize) {
        num_pieces = (file_size + piece_size - 1) / piece_size;
    }
    
    bool read_piece(uint32_t piece_index, std::vector<std::byte>& data) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return false;
        TODO
        return false;
    }
    
    bool write_piece(uint32_t piece_index, const std::vector<uint8_t>& data) {
        TODO
        return false;
    }
    
    bool create_empty_file() {
        std::ofstream file(filename, std::ios::binary);
        TODO
        return false;
    }
    
    size_t get_num_pieces() const { return num_pieces; }
    size_t get_piece_size() const { return piece_size; }
};

#endif
