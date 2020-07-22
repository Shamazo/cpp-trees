#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "btree.h"
#include "spdlog/spdlog.h"

TEST_CASE("check BNode binary search") {
    BNode* node = new BNode;
    for (size_t i =0; i < FAN_OUT; i++){
        node->children.vals[i] = i ;
//        0, 2, 4 , 6
        node->keys[i] = i * 2;
    }
    node->num_elements = FAN_OUT;
    std::optional<size_t> pos = node->binary_search(0, node->num_elements, 5);
    CHECK(pos.has_value() == true);
    CHECK(pos.value() == 2);

    pos = node->binary_search(0, node->num_elements, -1);
    CHECK(pos.has_value() == true);
    CHECK(pos.value() == 0);

    pos = node->binary_search(0, node->num_elements, 0);
    CHECK(pos.has_value() == true);
    CHECK(pos.value() == 0);
}

TEST_CASE("check simple one node BTree") {
    std::vector<int> keys;
    for (int i = 0; i < 10; i++) {
        keys.push_back(i * 5);
    }
    std::vector<int> vals;
    for (int i = 0; i < 10; i++) {
        vals.push_back(i);
    }
    Btree *tree = new Btree;
    tree->load(keys, vals);

    std::vector<int> res = tree->get_vals(5,6);
    std::vector<int> expected = {1};
    CHECK(res == expected);

    res = tree->get_vals(5,10);
    expected = {1};
    CHECK(res == expected);

    res = tree->get_vals(5,11);
    expected = {1, 2};
    CHECK(res == expected);
}

TEST_CASE("check two level BTree") {
    std::vector<int> keys;
    for (int i = 0; i < 200; i++) {
        keys.push_back(i * 5);
    }
    std::vector<int> vals;
    for (int i = 0; i < 200; i++) {
        vals.push_back(i);
    }
    Btree *tree = new Btree;
    tree->load(keys, vals);

    std::vector<int> res = tree->get_vals(50,56);
    std::vector<int> expected = {10, 11};
    CHECK(res == expected);
}


TEST_CASE("check big BTree") {
    std::vector<int> keys;
    for (int i = 0; i < 100000; i++) {
        keys.push_back(i * 5);
    }
    std::vector<int> vals;
    for (int i = 0; i < 100000; i++) {
        vals.push_back(i);
    }
    Btree *tree = new Btree;
    tree->load(keys, vals);

    std::vector<int> res = tree->get_vals(5,6);
    std::vector<int> expected = {1};
            CHECK(res == expected);

    res = tree->get_vals(0, 101);
    expected = {};
    for (int i=0; i < 21; i++){
        expected.push_back(i);
    }
    spdlog::info("result {}", fmt::join(res, ""));
    CHECK(res == expected);
}


TEST_CASE("check keys across leaves") {
    std::vector<int> keys;
    for (int i = 0; i < 100000; i++) {
        keys.push_back(i * 5);
    }
    std::vector<int> vals;
    for (int i = 0; i < 100000; i++) {
        vals.push_back(i);
    }
    Btree *tree = new Btree;
    tree->load(keys, vals);

    // fanout of 80, keys 0-400 in the leftmost leaf.
    std::vector<int> res = tree->get_vals(300, 501);
    std::vector<int> expected = {};
    for (int i=60; i < 101; i++){
        expected.push_back(i);
    }
    spdlog::info("result {}", fmt::join(res, ""));
            CHECK(res == expected);
}