// lzss.cpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-12
// Header file for the LZSS compression algorithm.

#ifndef LZSS_HPP
#define LZSS_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

size_t lzss_compress(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output);
size_t lzss_decompress(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output);

#endif
