// lzss.cpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-12
// Header file for the LZSS compression algorithm.

#ifndef LZSS_HPP
#define LZSS_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

// Compress the input data using LZSS algorithm
// input: pointer to the input data
// input_size: size of the input data
// output: vector to store the compressed data
// Returns the size of the compressed data
// If the output size exceeds the input size, compression failed
// and the function returns input_size, indicating no compression was done
// and the output vector is invalid
size_t lzss_compress(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output);

// Decompress the input data using LZSS algorithm
// input: pointer to the input data
// input_size: size of the input data
// output: vector to store the decompressed data
// Returns the size of the decompressed data
size_t lzss_decompress(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output);

#endif
