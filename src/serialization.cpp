// serialization.cpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-13
// Source file for the serialization functions, which handle reading and writing
// binary data based on scanning mode and transformation model.

#include <cstddef>
#include <cstdint>
#include <vector>
#include "lzss.hpp"
#include "serialization.hpp"

#define BLOCK_SIZE 32

void apply_difference(uint8_t* buffer, size_t width, size_t height) {
    for (size_t y = 0; y < height; y++) {
        uint8_t last_value = buffer[y * width];
        for (size_t x = 1; x < width; x++) {
            uint8_t current_value = buffer[y * width + x];
            buffer[y * width + x] = current_value - last_value;
            last_value = current_value;
        }
    }
}

void remove_difference(uint8_t* buffer, size_t width, size_t height) {
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 1; x < width; x++) {
            buffer[y * width + x] += buffer[y * width + x - 1];
        }
    }
}

size_t compress(uint8_t* input, size_t input_size, size_t width, bool adaptive, bool model, std::vector<uint8_t>& output) {
    output.reserve(input_size / 2);
    output.push_back(width / 256); // width byte [0]
    output.push_back(model ? 1 : 0); // model used flag [1]

    if (adaptive) {

    } else {
        if (model) {
            size_t height = input_size / width;
            apply_difference(input, width, height);
        }
        output.push_back(1); // block count [2]
        output.push_back(0x03); // scanning direction and been encoded flags [3]
        output.push_back(0); // placeholder for compressed size [4]
        output.push_back(0); // placeholder for compressed size [5]
        output.push_back(0); // placeholder for compressed size [6]
        output.push_back(0); // placeholder for compressed size [7]

        size_t compressed_size = lzss_compress(input, input_size, output);
        if (compressed_size == input_size) {
            output.resize(8);
            output.insert(output.end(), input, input + input_size);
            output[3] &= 0xFE; // clear the "been encoded" flag
        }
        output[4] = compressed_size & 0xFF;
        output[5] = (compressed_size >> 8) & 0xFF;
        output[6] = (compressed_size >> 16) & 0xFF;
        output[7] = (compressed_size >> 24) & 0xFF;
    }

    return output.size();
}

size_t decompress(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output) {
    if (input_size < 8) {
        return 0; // Input must be at least 8 bytes (single block)
    }

    size_t width = input[0] * 256;
    bool model = input[1] == 1;
    uint8_t block_count = input[2];

    output.reserve(input_size * 2);

    for (uint8_t i = 0; i < block_count; i++) {
        bool horizontal = input[3] & 0x02;
        bool been_encoded = input[3] & 0x01;
        size_t compressed_size = input[4] | (input[5] << 8) | (input[6] << 16) | (input[7] << 24);

        size_t decompressed_size = compressed_size;
        if (been_encoded) {
            decompressed_size = lzss_decompress(input + 8, compressed_size, output);
        } else {
            output.insert(output.end(), input + 8, input + 8 + compressed_size);
        }

        if (model) {
            size_t height = decompressed_size / width;
            remove_difference(output.data() + output.size() - decompressed_size, width, height);
        }
    }

    return output.size();
}
