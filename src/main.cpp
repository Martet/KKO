// main.cpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-11
// Program entry point, handles command line arguments and file I/O

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include "includes/argparse.hpp"
#include "lzss.hpp"

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("lz_codec");
    auto &group = program.add_mutually_exclusive_group(true);
    group.add_argument("-c").help("Compression mode").flag();
    group.add_argument("-d").help("Decompression mode").flag();
    program.add_argument("-m").help("Activate preprocessing model").flag();
    program.add_argument("-a").help("Activate adaptive scanning mode").flag();
    program.add_argument("-w").help("Image width [required with -c]").scan<'i', int>().metavar("width_value");
    program.add_argument("-i").help("Input file").required().metavar("ifile");
    program.add_argument("-o").help("Output file").required().metavar("ofile");

    int width;
    bool compress;
    try {
        program.parse_args(argc, argv);
    
        compress = program.is_used("-c");
        if (compress) {
            width = program.get<int>("-w");
            if (width <= 0) {
                throw std::runtime_error("Error: Width must be a positive integer.");
            } else if (width % 256 != 0) {
                throw std::runtime_error("Error: Width must be a multiple of 256.");
            }
        }
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::string input_file = program.get<std::string>("-i");
    std::ifstream input_file_stream(input_file, std::ios::binary);
    if (!input_file_stream) {
        std::cerr << "Error: Could not open input file " << input_file << std::endl;
        return 1;
    }
    auto size = std::filesystem::file_size(input_file);
    if (compress && size / width % 256 != 0) {
        std::cerr << "Error: Input height must be a multiple of 256." << std::endl;
        return 1;
    }
    std::unique_ptr<uint8_t[]> input_buffer(new uint8_t[size]);
    input_file_stream.read(reinterpret_cast<char*>(input_buffer.get()), size);

    std::cout << "Input file size: " << size << " bytes" << std::endl;

    std::string output_file = program.get<std::string>("-o");
    std::ofstream output_file_stream(output_file, std::ios::binary);
    if (!output_file_stream) {
        std::cerr << "Error: Could not open output file " << output_file << std::endl;
        return 1;
    }
    
    if (compress) {
        std::unique_ptr<uint8_t[]> output_buffer(new uint8_t[size]);
        std::cout << "Compressed file size: " << lzss_compress(input_buffer.get(), size, output_buffer.get(), 0) << " bytes" << std::endl;
        output_file_stream.write(reinterpret_cast<char*>(output_buffer.get()), size);
    } else {
        std::unique_ptr<uint8_t[]> output_buffer(new uint8_t[262144]);
        size_t decompressed_size = lzss_decompress(input_buffer.get(), size, output_buffer.get(), 262144);
        if (decompressed_size == 0) {
            std::cerr << "Error: Decompression failed." << std::endl;
            return 1;
        }
        output_file_stream.write(reinterpret_cast<char*>(output_buffer.get()), decompressed_size);
        std::cout << "Decompressed file size: " << decompressed_size << " bytes" << std::endl;
    }

    return 0;
}
