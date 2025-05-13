// serialization.hpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-13
// Header file for the serialization functions, which handle reading and writing
// binary data based on scanning mode and transformation model.

#ifndef SERIALIZATION_HPP
#define SERIALIZATION_HPP

#include <vector>
#include <cstdint>
#include <cstddef>

// Compress the input data
// input: pointer to the input data read from file
// input_size: size of the input data
// width: width of the image from the command line
// adaptive: true if adaptive scanning mode is used
// model: true if the preprocessing model is used
// output: vector to store the compressed data to be written to file
// Returns the size of the compressed data 
size_t compress(uint8_t* input, size_t input_size, size_t width, bool adaptive, bool model, std::vector<uint8_t>& output);

// Decompress the input data
// input: pointer to the input data read from file
// input_size: size of the input data
// output: vector to store the decompressed data to be written to file
// Returns the size of the decompressed data, 0 if input is invalid
//
// The function will also handle the adaptive scanning mode and the preprocessing model,
// these are read from the input data header
size_t decompress(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output);

#endif
