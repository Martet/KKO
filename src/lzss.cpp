// lzss.cpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-12
// Source file for the LZSS compression algorithm.

#include <cstddef>
#include <cstdint>
#include "lzss_buffer.hpp"
#include "lzss.hpp"

size_t lzss_compress(const uint8_t* input, size_t input_size, uint8_t* output, size_t output_size) {
    size_t output_written = 0;
    uint8_t header_index = 0, tag_index = 0, flags_byte = 0;
    uint8_t tag_buffer[16];

    SearchBuffer search_buffer(input, input_size);

    for (size_t i = 0; i < input_size; i++) {
        size_t match_len = 0;
        size_t match_pos = search_buffer.find_best_match(i, &match_len);
        if (match_len >= MATCH_THRESHOLD) {
            uint16_t out_tag = ((i - match_pos) << 4) | (match_len - MATCH_THRESHOLD);
            tag_buffer[tag_index++] = out_tag & 0xFF;
            tag_buffer[tag_index++] = (out_tag >> 8) & 0xFF;
            flags_byte |= (1 << header_index++);
            i += match_len - 1;
            search_buffer.slide(match_len);
        } else {
            tag_buffer[tag_index++] = input[i];
            flags_byte |= (1 << header_index++);
            search_buffer.slide(1);
        }

        if (header_index == 8 || i == input_size - 1) {
            if (output_written + 1 + tag_index >= input_size) {
                return input_size; // Output too large, compression failed
            }
            
            output[output_written++] = flags_byte;
            for (size_t j = 0; j < tag_index; j++) {
                output[output_written++] = tag_buffer[j];
            }
            flags_byte = 0;
            header_index = 0;
            tag_index = 0;
        }
    }

    return output_written;
}

size_t lzss_decompress(const uint8_t* input, size_t input_size, uint8_t* output, size_t output_size) {
    size_t output_written = 0;
    size_t input_pos = 0;

    while (input_pos < input_size) {
        uint8_t flags_byte = input[input_pos++];
        for (int i = 0; i < 8 && input_pos < input_size; i++) {
            if (flags_byte & (1 << i)) {
                if (output_written >= output_size) {
                    return output_size; // Output buffer too small
                }
                output[output_written++] = input[input_pos++];
            } else {
                uint16_t match_offset = input[input_pos++] | (input[input_pos++] << 8);
                size_t match_len = (match_offset & 0x0F) + MATCH_THRESHOLD;
                size_t match_pos = output_written - ((match_offset >> 4) + 1);

                for (size_t j = 0; j < match_len; j++) {
                    if (output_written >= output_size) {
                        return output_size; // Output buffer too small
                    }
                    output[output_written++] = output[match_pos++];
                }
            }
        }
    }

    return output_written;
}
