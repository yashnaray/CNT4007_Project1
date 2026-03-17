#pragma once
#ifndef CNT4007_PROJECT1_FILEMANAGER_H
#define CNT4007_PROJECT1_FILEMANAGER_H

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cstddef>
#include <cstdint>

class FileManager {
    std::string filename;
    size_t file_size;
    size_t piece_size;
    size_t num_pieces;
    
public:
    FileManager(const std::string& fname, size_t fsize, size_t psize): filename(fname), file_size(fsize), piece_size(psize) {
        if (piece_size > 0) {
            num_pieces = (file_size + piece_size - 1) / piece_size;
        } else {
            num_pieces = 0;
        }
    }
    
    bool read_piece(uint32_t piece_index, std::vector<uint8_t>& data) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return false;
        
        size_t offset = piece_index * piece_size;
        size_t current_piece_size = piece_size;
        if (piece_index == num_pieces - 1 && file_size % piece_size != 0) {
            current_piece_size = file_size % piece_size;
        }
        
        file.seekg(offset, std::ios::beg);
        data.resize(current_piece_size);
        file.read(reinterpret_cast<char*>(data.data()), current_piece_size);
        return file.good() || file.eof();
    }
    
    bool write_piece(uint32_t piece_index, const std::vector<uint8_t>& data) {
        std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) return false;
        
        size_t offset = piece_index * piece_size;
        file.seekp(offset, std::ios::beg);
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return file.good();
    }
    
    bool create_empty_file() {
        std::ofstream file(filename, std::ios::binary);
        if (!file) return false;
        
        if (file_size > 0) {
            file.seekp(file_size - 1);
            file.write("", 1);
        }
        return true;
    }
    
    size_t get_num_pieces() const { return num_pieces; }
    size_t get_piece_size() const { return piece_size; }
};

#endif