/**
 * @file btree.h
 *
 * This file defines the Btree class.
 */
#ifndef BTREE_H
#define BTREE_H

#include <cstddef>
#include <vector>
#include <optional>

const int FAN_OUT = 80;

class BNode{
public:
    union {
        size_t vals[FAN_OUT + 1];
        struct BNode* child_pointers[FAN_OUT+1];
    } children{};
    int keys[FAN_OUT] = {0};
    size_t num_elements = 0;
    BNode* previous;
    BNode* next;
    bool is_leaf;
    bool is_root;
    BNode();
    std::optional<size_t> binary_search(size_t lk, size_t rk, int key);
};

class Btree {
public:
    Btree();
    void load(std::vector<int> keys, std::vector<int> values);
    void insert(int key, int value);
    void traverse_tree(void (*func)(BNode*));
    std::vector<int> get_vals(int lower, int upper);
private:
    void split_node(BNode* bNode);
    BNode* build_leaves(std::vector<int> keys, std::vector<int> values);
    std::pair<BNode*, size_t> build_layer(BNode* head);
    BNode* root;
};

void log_node(BNode* bnode);


#endif  // BTREE_H