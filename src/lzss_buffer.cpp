// lzss_buffer.cpp
// Author: Martin Zmitko (xzmitk01), created on 2025-05-12
// Source file for the SearchBuffer class, which implements a binary search
// tree and manages a sliding window buffer keeping the tree updated.

#include <cstddef>
#include <cstdint>
#include "lzss_buffer.hpp"

SearchBuffer::SearchBuffer(const uint8_t* buffer, size_t buffer_size)
    : buffer(buffer), buffer_size(buffer_size), window_pos(0), root(nullptr)
    {}

void SearchBuffer::slide(size_t n) {
    for (size_t i = window_pos; i < window_pos + n; i++) {
        if (i >= buffer_size) {
            break;
        }

        if (i >= SLIDING_WINDOW_SIZE) {
            root = delete_node(root, i - SLIDING_WINDOW_SIZE);
        }

        window_pos++;
        root = insert_node(root, i);
    }
}

size_t SearchBuffer::find_best_match(size_t pos, size_t* match_len) const {
    return best_match_node(root, pos, match_len);
}

SearchBuffer::~SearchBuffer() {
    delete_tree(root);
    root = nullptr;
}

void SearchBuffer::delete_tree(Node* node) {
    if (!node)
        return;

    delete_tree(node->left);
    delete_tree(node->right);
    delete node;
}

SearchBuffer::Node* SearchBuffer::insert_node(Node* node, size_t pos) {
    if (!node)
        return new Node(pos);

    if (compare(pos, node->pos) < 0) {
        node->left = insert_node(node->left, pos);
    } else {
        node->right = insert_node(node->right, pos);
    }

    return node;
}

SearchBuffer::Node* SearchBuffer::delete_node(Node* node, size_t pos) {
    if (!node)
        return nullptr;

    int16_t cmp = compare(pos, node->pos);
    if (cmp < 0) {
        node->left = delete_node(node->left, pos);
    } else if (cmp > 0) {
        node->right = delete_node(node->right, pos);
    } else {
        if (!node->left) {
            Node* right = node->right;
            delete node;
            return right;
        } else if (!node->right) {
            Node* left = node->left;
            delete node;
            return left;
        } else {
            Node* succ_parent = node;
            Node* succ = node->right;
            while (succ->left) {
                succ_parent = succ;
                succ = succ->left;
            }
            node->pos = succ->pos;
            if (succ_parent->left == succ) {
                succ_parent->left = succ->right;
            } else {
                succ_parent->right = succ->right;
            }
            delete succ;
        }
    }

    return node;
}

size_t SearchBuffer::best_match_node(Node* node, size_t pos, size_t* best_len) const {
    if (!node)
        return SIZE_MAX;

    size_t match_len = common_prefix_len(pos, node->pos);
    size_t best_pos = SIZE_MAX;

    if (match_len > *best_len) {
        *best_len = match_len;
        best_pos = node->pos;
    }

    if (compare(pos, node->pos) < 0) {
        size_t left = best_match_node(node->left, pos, best_len);
        if (left != SIZE_MAX) {
            best_pos = left;
        }
    } else {
        size_t right = best_match_node(node->right, pos, best_len);
        if (right != SIZE_MAX) {
            best_pos = right;
        }
    }

    return best_pos;
}

int16_t SearchBuffer::compare(size_t pos_a, size_t pos_b) const {
    for (size_t i = 0; i < LOOKAHEAD_SIZE; i++) {
        if (pos_a + i >= buffer_size || pos_b + i >= buffer_size) {
            break;
        } else if (buffer[pos_a + i] != buffer[pos_b + i]) {
            return buffer[pos_a + i] - buffer[pos_b + i];
        }
    }

    return 0;
}

size_t SearchBuffer::common_prefix_len(size_t pos_a, size_t pos_b) const {
    for (size_t i = 0; i < LOOKAHEAD_SIZE; i++) {
        if (pos_a + i >= buffer_size || pos_b + i >= buffer_size
            || buffer[pos_a + i] != buffer[pos_b + i]
        ) {
            return i + 1;
        }
    }

    return LOOKAHEAD_SIZE;
}

