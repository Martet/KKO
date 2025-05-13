// serialization.cpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-13
// Source file for the serialization functions, which handle reading and writing
// binary data based on scanning mode and transformation model.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include "lzss.hpp"
#include "serialization.hpp"

#define BLOCK_SIZE 64
#define BLOCK_BYTE_SIZE (BLOCK_SIZE * BLOCK_SIZE)

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

void transpose_block(uint8_t* buffer) {
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        for (size_t j = i + 1; j < BLOCK_SIZE; j++) {
            std::swap(buffer[i * BLOCK_SIZE + j], buffer[j * BLOCK_SIZE + i]);
        }
    }
}

size_t compress(uint8_t* input, size_t input_size, size_t width, bool adaptive, bool model, std::vector<uint8_t>& output) {
    output.reserve(input_size / 2);
    output.push_back(width / 256); // block width byte [0]
    output.push_back(model ? 1 : 0); // model used flag [1]

    if (adaptive) {
        uint8_t horizontal_block[BLOCK_BYTE_SIZE];
        uint8_t vertical_block[BLOCK_BYTE_SIZE];
        std::vector<uint8_t> horizontal_output;
        horizontal_output.reserve(BLOCK_BYTE_SIZE);
        std::vector<uint8_t> vertical_output;
        vertical_output.reserve(BLOCK_BYTE_SIZE);
        
        size_t height = input_size / width;
        size_t block_count = (width / BLOCK_SIZE) * (height / BLOCK_SIZE);
        output.push_back((block_count >> 8) & 0xff); // block count upper [3]
        output.push_back(block_count & 0xff); // block count lower [2]
        
        for (size_t y = 0; y < height; y += BLOCK_SIZE) {
            for (size_t x = 0; x < width; x += BLOCK_SIZE) {
                output.push_back(0); // scanning direction and been encoded flags placeholder

                for (size_t by = 0; by < BLOCK_SIZE; by++) {
                    memcpy(
                        horizontal_block + by * BLOCK_SIZE,
                        input + (y + by) * width + x,
                        std::min((size_t)BLOCK_SIZE, width - x)
                    );
                }
                memcpy(vertical_block, horizontal_block, BLOCK_BYTE_SIZE);
                transpose_block(vertical_block);

                if (model) {
                    apply_difference(horizontal_block, BLOCK_SIZE, BLOCK_SIZE);
                    apply_difference(vertical_block, BLOCK_SIZE, BLOCK_SIZE);
                }

                size_t horizontal_size = lzss_compress(horizontal_block, BLOCK_BYTE_SIZE, horizontal_output);
                size_t vertical_size = lzss_compress(vertical_block, BLOCK_BYTE_SIZE, vertical_output);
                
                size_t compressed_size = std::min(horizontal_size, vertical_size);
                output.push_back(compressed_size & 0xFF);
                output.push_back((compressed_size >> 8) & 0xFF);
                output.push_back((compressed_size >> 16) & 0xFF);
                output.push_back((compressed_size >> 24) & 0xFF);

                if (horizontal_size < vertical_size) {
                    output[output.size() - 5] = 0x03;
                    output.insert(output.end(), horizontal_output.begin(), horizontal_output.begin() + horizontal_size);
                } else if (vertical_size != BLOCK_BYTE_SIZE) {
                    output[output.size() - 5] = 0x01;
                    output.insert(output.end(), vertical_output.begin(), vertical_output.begin() + vertical_size);
                } else {
                    output[output.size() - 5] = 0x02;
                    output.insert(output.end(), horizontal_block, horizontal_block + BLOCK_BYTE_SIZE);
                }

                horizontal_output.clear();
                vertical_output.clear();
            }
        }
    } else {
        if (model) {
            size_t height = input_size / width;
            apply_difference(input, width, height);
        }
        output.push_back(0); // block count lower [2]
        output.push_back(1); // block count upper [3]
        output.push_back(0x03); // scanning direction and been encoded flags [4]
        output.push_back(0); // placeholder for compressed size [5]
        output.push_back(0); // placeholder for compressed size [6]
        output.push_back(0); // placeholder for compressed size [7]
        output.push_back(0); // placeholder for compressed size [8]

        size_t compressed_size = lzss_compress(input, input_size, output);
        if (compressed_size == input_size) {
            output.resize(9);
            output.insert(output.end(), input, input + input_size);
            output[4] &= 0xFE; // clear the "been encoded" flag
        }
        output[5] = compressed_size & 0xFF;
        output[6] = (compressed_size >> 8) & 0xFF;
        output[7] = (compressed_size >> 16) & 0xFF;
        output[8] = (compressed_size >> 24) & 0xFF;
    }

    return output.size();
}

size_t decompress(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output) {
    if (input_size < 9) {
        return 0; // Input must be at least 9 bytes (single block)
    }

    size_t width = input[0] * 256;
    bool model = input[1] == 1;
    size_t block_count = input[3] | (input[2] << 8);

    bool adaptive = block_count > 1;
    if (adaptive) {
        output.resize(block_count * BLOCK_BYTE_SIZE);
    }

    size_t curr_pos = 4;
    for (size_t i = 0; i < block_count; i++) {
        bool horizontal = input[curr_pos] & 0x02;
        bool been_encoded = input[curr_pos] & 0x01;
        size_t compressed_size = input[curr_pos + 1] | (input[curr_pos + 2] << 8) | (input[curr_pos + 3] << 16) | (input[curr_pos + 4] << 24);

        size_t decompressed_size = compressed_size;
        std::vector<uint8_t> block_output;
        std::vector<uint8_t>& output_ref = adaptive ? block_output : output;

        if (been_encoded) {
            decompressed_size = lzss_decompress(input + curr_pos + 5, compressed_size, output_ref);
        } else {
            output_ref.resize(compressed_size);
            std::copy(input + curr_pos + 5, input + curr_pos + 5 + compressed_size, output_ref.begin());
        }

        size_t height = adaptive ? BLOCK_SIZE : decompressed_size / width;
        if (model) {
            remove_difference(output_ref.data() + output_ref.size() - decompressed_size, adaptive ? BLOCK_SIZE : width, height);
        }

        if (!horizontal) {
            transpose_block(output_ref.data() + output_ref.size() - decompressed_size);
        }

        // Copy block to the correct position in output
        if (adaptive) {
            size_t block_x = (i % (width / BLOCK_SIZE)) * BLOCK_SIZE;
            size_t block_y = (i / (width / BLOCK_SIZE)) * BLOCK_SIZE;

            for (size_t y = 0; y < BLOCK_SIZE; y++) {
                std::copy(
                    block_output.begin() + y * BLOCK_SIZE,
                    block_output.begin() + (y + 1) * BLOCK_SIZE,
                    output.begin() + (block_y + y) * width + block_x 
                );
            }
        } // otherwise the output is already in output

        curr_pos += compressed_size + 5;
    }

    return output.size();
}
