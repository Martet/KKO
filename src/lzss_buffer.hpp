// lzss_buffer.hpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-12
// Header file for the SearchBuffer class, which implements a binary search
// tree and manages a sliding window buffer keeping the tree updated.

#ifndef LZSS_TREE_HPP
#define LZSS_TREE_HPP

#include <cstddef>
#include <cstdint>

#define SLIDING_WINDOW_SIZE 4096
#define LOOKAHEAD_SIZE 18
#define MATCH_THRESHOLD 3

class SearchBuffer {
public:
    SearchBuffer(const uint8_t* buffer, size_t buffer_size);

    void slide(size_t n);

    size_t find_best_match(size_t pos, size_t* match_len) const;

    ~SearchBuffer();

private:
    struct Node {
        size_t pos;
        Node* left = nullptr;
        Node* right = nullptr;
        Node(size_t pos) : pos(pos) {}
    };

    const uint8_t* buffer;
    size_t buffer_size, window_pos;
    Node* root;

    void delete_tree(Node* node);

    Node* insert_node(Node* node, size_t pos);

    Node* delete_node(Node* node, size_t pos);

    size_t best_match_node(Node* node, size_t pos, size_t* best_len) const;

    int16_t compare(size_t a, size_t b) const;

    size_t common_prefix_len(size_t a, size_t b) const;
};

#endif
