// serialization.hpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-13
// Header file for the serialization functions, which handle reading and writing
// binary data based on scanning mode and transformation model.

#ifndef SERIALIZATION_HPP
#define SERIALIZATION_HPP

#include <vector>
#include <cstdint>
#include <cstddef>

size_t compress(uint8_t* input, size_t input_size, size_t width, bool adaptive, bool model, std::vector<uint8_t>& output);
size_t decompress(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output);

#endif
