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
#include <vector>
#include "includes/argparse.hpp"
#include "serialization.hpp"

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

    int width = 0;
    bool compress_flag;
    try {
        program.parse_args(argc, argv);
    
        compress_flag = program.is_used("-c");
        if (compress_flag) {
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
    if (compress_flag && size / width % 256 != 0) {
        std::cerr << "Error: Input height must be a multiple of 256." << std::endl;
        return 1;
    }
    std::unique_ptr<uint8_t[]> input_buffer(new uint8_t[size]);
    input_file_stream.read(reinterpret_cast<char*>(input_buffer.get()), size);

    std::string output_file = program.get<std::string>("-o");
    std::ofstream output_file_stream(output_file, std::ios::binary);
    if (!output_file_stream) {
        std::cerr << "Error: Could not open output file " << output_file << std::endl;
        return 1;
    }
    
    size_t output_size;
    std::vector<uint8_t> output_buffer;
    output_buffer.reserve(size);
    if (compress_flag) {
        output_size = compress(input_buffer.get(), size, width, program.is_used("-a"), program.is_used("-m"), output_buffer);
    } else {
        output_size = decompress(input_buffer.get(), size, output_buffer);
        if (output_size == 0) {
            std::cerr << "Error: Decompression failed." << std::endl;
            return 1;
        }
    }
    output_file_stream.write(reinterpret_cast<char*>(output_buffer.data()), output_size);

    #ifdef DEBUG
    std::cout << "Input file size: " << size << " bytes" << std::endl;
    std::cout << "Output file size: " << output_size << " bytes" << std::endl;
    #endif

    return 0;
}
