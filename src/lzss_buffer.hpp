// lzss_buffer.hpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-12
// Header file for the SearchBuffer class, which implements a binary search
// tree and manages a sliding window buffer keeping the tree updated.

#ifndef LZSS_TREE_HPP
#define LZSS_TREE_HPP

#include <cstddef>
#include <cstdint>

#define SLIDING_WINDOW_SIZE 2048
#define LOOKAHEAD_SIZE 34
#define MATCH_THRESHOLD 3

class SearchBuffer {
public:
    SearchBuffer(const uint8_t* buffer, size_t buffer_size);

    // Slide the window by n bytes, updating the binary search tree
    // to reflect the new positions of the data
    void slide(size_t n);

    // Find the best match for the current position in the buffer
    // Returns the position of the best match and updates match_len
    // with the length of the match
    // If no match is found, returns SIZE_MAX
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

    // Delete the binary search tree starting from the given node
    void delete_tree(Node* node);

    // Insert a new node into the binary search tree
    // Returns the new root of the tree
    Node* insert_node(Node* node, size_t pos);

    // Delete a node from the binary search tree
    // Returns the new root of the tree
    Node* delete_node(Node* node, size_t pos);

    // Find the best match node in the binary search tree
    // starting from the given node
    // Returns the position of the best match and updates best_len
    // with the length of the match
    size_t best_match_node(Node* node, size_t pos, size_t* best_len) const;

    // Compare two positions in the buffer
    // Returns a negative value if a < b, 0 if a == b, and a positive value if a > b
    // The comparison is done byte by byte, up to LOOKAHEAD_SIZE bytes
    int16_t compare(size_t a, size_t b) const;

    // Find the length of the common prefix between two positions in the buffer
    // Returns the length of the common prefix
    // The comparison is done byte by byte, up to LOOKAHEAD_SIZE bytes
    size_t common_prefix_len(size_t a, size_t b) const;
};

#endif
