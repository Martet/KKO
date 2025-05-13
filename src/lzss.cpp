// lzss.cpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-12
// Source file for the LZSS compression algorithm.

#include <cstddef>
#include <cstdint>
#include <vector>
#include "lzss_buffer.hpp"
#include "lzss.hpp"

size_t lzss_compress(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output) {
    uint8_t flags_index = 0, flags_byte = 0;
    size_t wrote = 0;
    std::vector<uint8_t> tag_buffer;

    SearchBuffer search_buffer(input, input_size);

    for (size_t i = 0; i < input_size; i++) {
        size_t match_len = 0;
        size_t match_pos = search_buffer.find_best_match(i, &match_len);
        if (match_len >= MATCH_THRESHOLD) {
            uint8_t out_len = match_len - MATCH_THRESHOLD;
            uint16_t out_pos = (i - match_pos - 1) << 4;
            uint16_t out_tag = out_len | out_pos;
            tag_buffer.push_back(out_tag & 0xFF);
            tag_buffer.push_back(out_tag >> 8);
            flags_byte |= (1 << flags_index); // Flag == 1 for a tag
            i += match_len - 1;
            search_buffer.slide(match_len);
        } else {
            tag_buffer.push_back(input[i]);
            search_buffer.slide(1);
        }
        flags_index++;

        // If we have 8 flags or reached the end of input, write the flags and tags
        if (flags_index == 8 || i == input_size - 1) {
            size_t to_write = tag_buffer.size() + 1;
            if (wrote + to_write >= input_size) {
                return input_size; // Output too large, compression failed
            }
            
            output.push_back(flags_byte);
            for (size_t j = 0; j < tag_buffer.size(); j++) {
                output.push_back(tag_buffer[j]);
            }
            tag_buffer.clear();
            wrote += to_write;
            flags_byte = 0;
            flags_index = 0;
        }
    }

    return wrote;
}

size_t lzss_decompress(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output) {
    size_t input_pos = 0, wrote = 0;

    while (input_pos < input_size) {
        uint8_t flags_byte = input[input_pos++];
        for (int i = 0; i < 8 && input_pos < input_size; i++) {
            if (flags_byte & (1 << i)) {
                uint8_t low_byte = input[input_pos++];
                uint8_t high_byte = input[input_pos++];
                uint16_t tag = (high_byte << 8) | low_byte;
                size_t match_len = (tag & 0x0F) + MATCH_THRESHOLD;
                size_t match_pos = output.size() - (tag >> 4) - 1;

                for (size_t j = 0; j < match_len; j++) {
                    output.push_back(output[match_pos++]);
                }
                wrote += match_len;
            } else {
                output.push_back(input[input_pos++]);
                wrote++;
            }
        }
    }

    return wrote;
}
